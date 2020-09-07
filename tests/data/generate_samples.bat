SET MAGICK_HOME=C:\Users\viscdari\Downloads\ImageMagick-7.0.8-7-portable-Q16-x64

%MAGICK_HOME%\convert.exe -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:rows-per-strip=128 -depth 8 TIFF:grad1024_scanline_8bpp_32bit.tif
%MAGICK_HOME%\convert -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:tile-geometry=128x128 -depth 8 TIFF:grad1024_tiled_8bpp_32bit.tif

%MAGICK_HOME%\convert.exe -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:rows-per-strip=128 -depth 16 TIFF:grad1024_scanline_16bpp_32bit.tif
%MAGICK_HOME%\convert -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:tile-geometry=128x128 -depth 16 TIFF:grad1024_tiled_16bpp_32bit.tif

:: Not supported by ImageMagick :-(
:: %MAGICK_HOME%\convert.exe -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:rows-per-strip=128 -depth 8 TIFF64:grad1024_scanline_8bpp_64bit.tif
:: %MAGICK_HOME%\convert -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:tile-geometry=128x128 -depth 8 TIFF64:grad1024_tiled_8bpp_64bit.tif

:: %MAGICK_HOME%\convert.exe -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:rows-per-strip=128 -depth 16 TIFF64:grad1024_scanline_16bpp_64bit.tif
:: %MAGICK_HOME%\convert -size 1024x1024 gradient: -size 512x512 gradient: -define tiff:tile-geometry=128x128 -depth 16 TIFF64:grad1024_tiled_16bpp_64bit.tif