"""
Loads the image with all the sprites, then saves each connected component in its
own RGB(no alpha) PNG file
"""

import warnings
from pathlib import Path
from PIL import Image

import numpy as np


class SplitImages:
    """
    Used to split an image into shapes-- its connected components, with an edge
    between any two non-transparent pixels

    Transparency can be configured so a specific pixel value is treated as
    transparent, or the alpha layer can be used for images with an alpha
    """

    DIRS_NO_CORNERS = ((1, 0), (0, 1), (-1, 0), (0, -1))
    DIRS_WITH_CORNERS = DIRS_NO_CORNERS + ((1, -1), (1, 1), (-1, 1), (-1, -1))

    def __init__(self, path, backgroundColor=None, backgroundLoc=None):

        if isinstance(path, Path):
            self.path = path
        else:
            self.path = path = Path(path)

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
        """
        Attempts to read the image by resolving `self.path`

        Stores the PIL `Image` in `self.image`, and the pixel data in
        `self.pixels`

        Sets `self.isTransparent` to whether the the image has a transparency
        layer; if the image doesn't, then one of `backgroundColor` or
        `backgroundLoc` must be non-`None`; otherwise, raises a `ValueError`
        """

        # read image
        self.image = Image.open(self.path.resolve())

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

    def pixelIsTransparent(self, pixel):
        """
        Checks `pixel`'s transparency based on equality with `self.bgColor`,
        then if `self.isTransparent`, based on the alpha channel
        """
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
        """
        Returns `True` iff `readImg` was sucessful, and there's a PIL `Image`
        object
        """
        return (
            self.image is not None
            and self.pixels is not None
            and isinstance(self.image, Image.Image)
        )

    def transpCheck(self):
        """
        Returns `True` iff there is a way to tell if a pixel is transparent--
        that is, by equality with `self.bgColor`, or with an alpha channel
        """
        return self.bgColor is not None or self.isTransparent is True

    def getPixel(self, row: int, col: int):
        """
        Returns the value of the pixel at location `(row, col)`

        Raises a `ValueError` if `readImg` hasn't been called

        Raises an `IndexError` if the pixel isn't within the size of the read
        image
        """
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
        """
        Adds each shape (connected component) in the image to `self.shapes`

        If `corners` is `True`, then pixels which border diagonally will be
        considered touching, part of the same shape
        """
        self.__check()

        seen = set()
        width, height = self.image.width, self.image.height

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
                self.__getConnectedPixelsBFS(
                    seen, (row, col), newShape, corners
                )

                # pixels returned by getConnectedPixels have been added to seen
                self.shapes.append(newShape)

    def __getConnectedPixelsBFS(
        self,
        seen: set[tuple[int, int]],
        loc: tuple[int, int],
        shape: list[tuple[int, int]],
        corners=True,
    ):

        if loc in seen:
            return

        # can't add transparent pixels to shape
        assert not self.__pixelIsTransparent(self.__getPixel(*loc))

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
                    newLoc = row + rowOff, col + colOff
                    if newLoc in seen:
                        continue

                    try:
                        pixel = self.__getPixel(*newLoc)
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

    def saveShapes(self, outputDir: Path, outBackgroudColor=None):
        """
        For each shape in `self.shapes`, save a RGB PNG of that shape

        Pixels which aren't in the shape are replaced with `backgroundColor` if
        its non-`None`; otherwise, `self.bgColor` is used
        """
        self.__check()

        if outBackgroudColor is None:
            outBackgroudColor = self.bgColor[:3]  # R, G, B

        outputDir.mkdir(exist_ok=True, parents=True)

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

            if shapeWidth < 1 or shapeHeight < 1:
                warnings.warn(f"Tried to save zero dim shape:{shape}")
                continue

            # make a new RNG-PNG (no alpha) image, load shape data
            newImage = Image.new(
                "RGB",
                (shapeWidth, shapeHeight),
                color=tuple(outBackgroudColor),
            )

            for row, col in shape:
                newRow, newCol = (row - minRow, col - minCol)
                newImage.putpixel(
                    (newCol, newRow), self.image.getpixel((col, row))[:3]
                )

            (arbRow, arbCol) = shape[0]
            name = f"shape_{arbRow}_{arbCol}.png"
            newImage.save(outputDir.joinpath(name))

    def __repr__(self):
        bgColorStr = (
            f", backgroundColor={self.bgColor}"
            if self.bgColor is not None
            else ""
        )

        bgLocStr = (
            f", backgroundLoc={self.bgLoc}" if self.bgLoc is not None else ""
        )

        return f"SplitImages({repr(self.path)}{bgColorStr}{bgLocStr})"


def main():
    """
    Opens `Derek_and_Harum1_Sprites.png`, then saves all shapes in it
    """
    path = Path("..", "media", "raw", "Derek_and_Harum1_Sprites.png")
    absolute_path = path.resolve()
    print("absolute path to sprites", absolute_path)

    splitImg = SplitImages(path, backgroundLoc=(1, 1))
    print("reading image")
    splitImg.readImg()

    splitImg.getShapes()
    print(f"Found {len(splitImg.shapes)} shapes")

    save_path = Path(
        "..", "media", "shapes", "Derek_and_Harum1_Sprites_shapes"
    )
    print("Saving shapes to", save_path.absolute())
    splitImg.saveShapes(save_path)
    print("Saved shapes")


if __name__ == "__main__":
    main()
