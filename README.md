# Videomation encoder

The video-information encoder converts any file to images (using every 3-bytes chunk as a pixel), and combines them into a video.

## ffmpeg commands

### Generate video from images
`ffmpeg -y -framerate 30 -i image_0000.bmp -c:v libx264rgb -vf "fps=30" -crf 0 -preset veryslow -pix_fmt bgr24 output.mp4`
```
-c:v libx264rgb: Uses the RGB color space for lossless encoding.
-crf 0: Ensures lossless quality.
-preset veryslow: Use the highest quality settings.
-pix_fmt bgr24: Matches the BMP format's pixel format.
```

### Generate images from video
`ffmpeg -i output.mp4 -vf "fps=30" -vsync 0 -pix_fmt bgr24 extracted_%04d.bmp`
```
-vsync 0: Extract frames without duplicate or drop any frame.
-pix_fmt bgr24: Matches the BMP format's pixel format.
```

References:

- https://shotstack.io/learn/use-ffmpeg-to-convert-images-to-video/
- https://www.bannerbear.com/blog/how-to-extract-images-from-a-video-using-ffmpeg/
- ChatGPT (for the extra parameters to ensure the extracted frames matches the original images to construct the video).
