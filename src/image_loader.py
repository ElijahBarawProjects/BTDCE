import cProfile
import pstats
import sys
import warnings
from pathlib import Path
from PIL import Image, UnidentifiedImageError

import numpy as np


class SplitImages:
    DIRS_NO_CORNERS = ((1, 0), (0, 1), (-1, 0), (0, -1))
    DIRS_WITH_CORNERS = DIRS_NO_CORNERS + ((1, -1), (1, 1), (-1, 1), (-1, -1))

    def __init__(self, path: Path, backgroundColor=None, backgroundLoc=None):

        self.path = path
        self.absPath = path.resolve()

        # the color to treat as transparent
        self.bgColor = backgroundColor

        # a location of a pixel which we treat as the background color
        self.bgLoc = backgroundLoc

        # will be set by readImg()
        self.image: Image.Image = None
        self.isTransparent = self.pixels = None

        # will be set by getShapes()
        self.shapes = []

    def readImg(self):
        try:
            # read image
            self.image = Image.open(self.absPath)

            # get pixels as np array
            self.pixels = np.array(self.image)

            # get a way to tell if a pixel is transparent
            self.isTransparent = self.image.mode in ("RGBA", "LA") or (
                self.image.mode == "P" and "transparency" in self.image.info
            )

            if self.bgColor is not None:
                return  # pixel is transparent if it matches bgColor

            if self.bgLoc is not None:
                # pixel is transparent if it matches the pixel at bgLoc
                row, col = self.bgLoc
                self.bgColor = self.__getPixel(row, col)
                return

            if self.isTransparent:
                return  # pixel will know if it's transparent

            # no way to tell if pixel is transparent
            raise ValueError("Unable to identify which pixels are transparent")

        except (
            FileNotFoundError,
            ValueError,
            UnidentifiedImageError,
            TypeError,
        ) as e:
            raise e

    def pixelIsTransparent(self, pixel):
        if not self.transpCheck():
            raise ValueError(
                "can't check transparency of non-transparent img without knowing bg color"
            )

        return self.__pixelIsTransparent(pixel)

    def __pixelIsTransparent(self, pixel):
        if self.bgColor is not None and np.array_equal(self.bgColor, pixel):
            return True

        if self.isTransparent:
            try:
                return bool(pixel[3] == 0)
            except (IndexError, ValueError) as e:
                print("No alpha channel")
                raise e

        return False

    def hasImg(self):
        return (
            self.image is not None
            and self.pixels is not None
            and isinstance(self.image, Image.Image)
        )

    def transpCheck(self):
        return self.bgColor is not None or self.isTransparent is True

    def getPixel(self, row: int, col: int):
        if not self.hasImg():
            raise ValueError(
                "Can't get pixel without image & img data",
                self.image,
                self.pixels,
            )
        return self.__getPixel(row, col)

    def __getPixel(self, row: int, col: int):
        # if out of bounds, numpy will let us know
        return self.pixels[row, col]

    def __check(self):
        if not (self.transpCheck() and self.hasImg()):
            raise ValueError(
                f"Failed check: {self.transpCheck}, {self.hasImg()}"
            )

    def getShapes(self, corners=True):
        self.__check()

        seen = set()
        width, height = self.image.width, self.image.height
        oldRecLimit = sys.getrecursionlimit()
        newRecLimit = width * height * 10
        sys.setrecursionlimit(newRecLimit)

        for row in range(height):
            for col in range(width):
                loc = (row, col)
                if loc in seen:
                    continue

                # don't try to make a shape from a transparent pixel
                if self.__pixelIsTransparent(self.__getPixel(row, col)):
                    seen.add(loc)
                    continue

                # find connected pixels
                newShape = []
                self.__getConnectedPixelsBFS(seen, row, col, newShape, corners)

                # pixels returned by getConnectedPixels have been added to seen
                self.shapes.append(newShape)

        sys.setrecursionlimit(oldRecLimit)

    def __getConnectedPixelsBFS(
        self,
        seen: set[tuple[int, int]],
        startRow: int,
        startCol: int,
        shape: list[tuple[int, int]],
        corners=True,
    ):

        loc = (startRow, startCol)
        if loc in seen:
            return

        # can't add transparent pixels to shape
        assert not self.__pixelIsTransparent(
            self.__getPixel(startRow, startCol)
        )

        dirs = (
            SplitImages.DIRS_WITH_CORNERS
            if corners
            else SplitImages.DIRS_NO_CORNERS
        )

        frontier = set()  # locations whose nbors we visit next
        frontier.add(loc)  # visit a pixel when putting it in the frontier
        shape.append(loc)
        seen.add(loc)

        while len(frontier) > 0:
            nextFrontier = set()
            for row, col in frontier:
                for rowOff, colOff in dirs:
                    newLoc = newRow, newCol = row + rowOff, col + colOff
                    if newLoc in seen:
                        continue

                    try:
                        pixel = self.__getPixel(newRow, newCol)
                    except IndexError:
                        continue

                    if self.__pixelIsTransparent(pixel):
                        seen.add(newLoc)
                        continue

                    # nbor hasn't been seen and is not transparent; visit it
                    nextFrontier.add(newLoc)
                    shape.append(newLoc)
                    seen.add(newLoc)
            frontier = nextFrontier

    def __getConnectedPixelsDFS(
        self,
        seen: set[tuple[int, int]],
        startRow: int,
        startCol: int,
        shape: list[tuple[int, int]],
        corners=True,
    ):
        """
        The pixel at (startRow, startCol) must not be transparent

        Adds (startRow, startCol) to seen, and to shape
        """

        loc = (startRow, startCol)
        if loc in seen:
            return

        # can't add transparent pixels to shape
        assert not self.__pixelIsTransparent(
            self.__getPixel(startRow, startCol)
        )

        dirs = (
            SplitImages.DIRS_WITH_CORNERS
            if corners
            else SplitImages.DIRS_NO_CORNERS
        )

        # visit this pixel
        shape.append(loc)
        seen.add(loc)

        for rowOff, colOff in dirs:
            newRow, newCol = startRow + rowOff, startCol + colOff
            newLoc = (newRow, newCol)
            if newLoc in seen:
                continue

            try:
                pixel = self.__getPixel(newRow, newCol)
            except IndexError:
                continue

            if self.__pixelIsTransparent(pixel):
                # visit pixel here, since it won't be visited in recursive call
                seen.add(newLoc)
                continue

            # continue the shape; also adds newLoc to shape and seen
            self.__getConnectedPixelsDFS(seen, newRow, newCol, shape, corners)

        return

    def saveShapes(self, outputDir: Path, backgroundColor=None):
        """
        For each shape in `self.shapes`, save a RGB PNG of that shape

        Pixels which aren't in the shape are replaced with `backgroundColor` if
        its non-`None`; otherwise, `self.bgColor` is used
        """
        self.__check()

        if backgroundColor is None:
            backgroundColor = self.bgColor[:3]  # R, G, B

        outputDir.mkdir(exist_ok=True)

        width, height = self.image.width, self.image.height
        for shape in self.shapes:

            if len(shape) == 0:
                warnings.warn(f"Tried to save zero dim shape:{shape}")
                continue

            # get the largest, smallest row, col
            maxRow = 0
            minRow = height
            maxCol = 0
            minCol = width
            for row, col in shape:
                maxRow = max(row, maxRow)
                minRow = min(row, minRow)
                maxCol = max(col, maxCol)
                minCol = min(col, minCol)

            # get the width, height, row offset, col offset
            shapeWidth = maxCol - minCol + 1
            shapeHeight = maxRow - minRow + 1
            rowOff = minRow  # will subtract from each row
            colOff = minCol  # will subtract from each col

            if shapeWidth < 1 or shapeHeight < 1:
                warnings.warn(f"Tried to save zero dim shape:{shape}")
                continue

            # make a new RNG-PNG (no alpha) image, load shape data
            newImage = Image.new(
                "RGB", (shapeWidth, shapeHeight), color=tuple(backgroundColor)
            )

            for row, col in shape:
                newRow, newCol = (row - rowOff, col - colOff)
                newImage.putpixel(
                    (newCol, newRow), self.image.getpixel((col, row))[:3]
                )

            (arbRow, arbCol) = shape[0]
            name = f"shape_{arbRow}_{arbCol}.png"

            newImage.save(outputDir.joinpath(name))


def main():

    profiler = cProfile.Profile()
    profiler.enable()

    path = Path("..", "media", "raw", "Derek_and_Harum1_Sprites.png")
    absolute_path = path.resolve()
    print("absolute path", absolute_path)

    splitImg = SplitImages(path, backgroundLoc=(1, 1))
    splitImg.readImg()

    pxl = splitImg.getPixel(2, 2)
    print(pxl, splitImg.pixelIsTransparent(pxl), splitImg.bgColor)

    print(len(splitImg.shapes))
    splitImg.getShapes()
    print(len(splitImg.shapes))

    splitImg.saveShapes(Path("..", "media", "Derek_and_Harum1_Sprites_shapes"))

    profiler.disable()
    stats = pstats.Stats(profiler)
    stats.sort_stats("time").print_stats(10)  # print top 10 functions by time

    # print("alt sprites")
    # path = Path("..", "media", "raw", "Reddit_Sprites_WillyOnCrack.jpg")
    # print("Initiating object")
    # splitImg = SplitImages(path, backgroundLoc=(1, 1))

    # print("Reading img")
    # splitImg.readImg()

    # print("Getting shapes")
    # splitImg.getShapes()

    # print("saving shapes")
    # splitImg.saveShapes(
    #     Path("..", "media", "Reddit_Sprites_WillyOnCrack_shapes")
    # )


if __name__ == "__main__":
    main()
