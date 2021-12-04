# SeaPrint
Network printing for Sailfish OS

![SeaPrint icon](icons/172x172/harbour-seaprint.png)

Print PDFs and images over IPP (Internet Printing Protocol).

Most, but not all, IPP-capable printers are supported.

A minority of printers support only PCL and/or their own proprietary formats, these are not supported.

## Format support

For PDFs or plaintext, your printer needs to support one of the following formats:
 * PDF
 * Postscript
 * PWG-raster
 * URF-raster

For Postscript, your printer needs to support Postscript natively. (SeaPrint just sends it as-is)

For printing "Office" files, like .doc(x) and odt, you need to install
 `calligraconverter` (from the `calligra` package in Sailfish OS 4.2 onwards).
 Then the printer support follows that of PDF.

For images, your printer needs to support one of the following formats:
 * PNG
 * JPEG
 * PWG-raster
 * URF-raster
 * PDF
 * Postscript

For printing JPEG images, transferring the unaltered image has priority and the printer does the scaling.
Other image formats are lossless and SeaPrint does scale-and-rotate to fit, so JPEG has lowest prio there.

Printers with any of these IPP certifications and derivative standards should likely be supported:

 * IPP Everywhere
 * AirPrint
 * Mopria
 * WiFi Direct

(obviously SeaPrint is unaffiliated and uncertified)
