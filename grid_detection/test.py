#import module to go through an image's pixels
from PIL import Image

#count number of white pixels in the image
def count_white_pixels(image):
    count = 0
    for pixel in image.getdata():
        if 255 in pixel:
            count += 1
    return count

def main():
    sum = 0
    n = 0
    min = 784
    #count the number of white pixels in each image in the folder test_digit
    for i in range(81):
        try:
            image = Image.open("test_digit/" + str(i) + ".bmp")
            nb_pixels = count_white_pixels(image)
            sum += nb_pixels
            n += 1
            if nb_pixels < min:
                min = nb_pixels
        except:
            pass
    print("sum =", sum)
    print("moyenne =", sum/n)
    print("plus basse =", min, "pixels blancs")

main()