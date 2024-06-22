

## ffmpeg command:
Generate video from images: `ffmpeg -framerate 30 -i images_%d.bmp -c:v libx264 -r 30 output.mp4`
Generate images from video: `ffmpeg -i input.mp4 %04d.bmp`

References:
- https://shotstack.io/learn/use-ffmpeg-to-convert-images-to-video/
- https://www.bannerbear.com/blog/how-to-extract-images-from-a-video-using-ffmpeg/
