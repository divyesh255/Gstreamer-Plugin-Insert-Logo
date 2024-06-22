
# GStreamer insert_logo

## Overview

The `insert_logo` GStreamer plugin allows you to overlay a logo onto a live video stream. It supports various features such as setting the logo position, rotating the logo, scrolling the logo horizontally, setting the speed of animations, and adjusting the transparency of the logo. it will just work for NV12 video frame format.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Plugin Parameters](#plugin-parameters)
- [Example Pipelines](#example-pipelines)
- [License](#license)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [Acknowledgements](#acknowledgements)

## Installation

Before you install the `insert_logo` Plugin, make sure you have GStreamer installed on your system. If not, you'll need to install it first. Here's a simplified guide to installing the plugin:

1. **Check GStreamer Installation**: Verify that GStreamer is already installed on your system. You can do this by running **`gst-inspect-1.0 --version`** in your terminal. If GStreamer is installed, you'll see the version number. If not, you'll need to install GStreamer before proceeding.

2. **Clone the Plugin Repository**: Clone the `insert_logo` plugin repository to your local machine. You can do this by running the following command in your terminal:
   ```bash
   git clone https://github.com/softnauticsgithub/GStreamer-Plugin-Development-InsertLogo.git
   ```

3. **Navigate to the Repository**: Change your current directory to the cloned repository. Use the following command:
   ```bash
   cd GStreamer-Plugin-Development-InsertLogo/InsertLogo_Divyesh/
   ```

4. **Build the Plugin**: Use meson to build the plugin. Run the following commands in your terminal:
   ```bash
   meson build
   ninja -C build
   ```

5. **Install the Plugin**: Once the plugin is built successfully, you can install it on your system. Run the following command in your terminal:
   ```bash
   cd build/gst-plugin/
   sudo cp libgstinsertlogo.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/
   ```
   
Replace **`/usr/lib/x86_64-linux-gnu/gstreamer-1.0/`** with the appropriate directory path if your system has a different location for GStreamer plugins.
   
## Usage

Once you've successfully installed the GStreamer `insert_logo` Plugin, you can incorporate it into your GStreamer pipelines. The plugin is identified in the pipeline as `insert_log`. You can utilize `gst-inspect-1.0` to display all the properties and details of the `insert_logo` plugin. To do this, simply type the following command in your terminal:

   ```bash
   gst-inspect-1.0 insert_logo
   ```

Below is an example demonstrating how to use the `insert_logo` Plugin in a GStreamer pipeline:

```console
gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1920,height=1080 ! insert_logo logo=<logo_image_path> scroll=LeftToRight speed=slow coordinate='<100,100>' ! autovideosink
```
In this example, replace `<logo_image_path>` with the file path of the PNG image you want to overlay on the video frames. You can also replace videotestsrc with other source elements like filesrc based on your input source. Additionally, you can adjust the animation, speed, opacity percentage, and other properties as needed for your specific use case.

```console
gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1920,height=1080 ! insert_logo logo=<logo_image_path> rotation=Clockwise speed=slow coordinate='<100,100>' alpha=50 ! autovideosink
```
In this example the `logo` parameter specifies the path to the logo image, `rotation` sets the rotation direction (Clockwise), `speed` determines the rotation speed (slow), `coordinate` defines the position of the logo (100,100) on the video frame and `alpha` parameter specifies the opacity level of the logo. Finally, the modified video is displayed using autovideosink.

```console
gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1920,height=1080 ! insert_logo logo=<logo_image_path> rotation=Clockwise speed=slow coordinate='<100,100>' alpha=50 ! autovideosink
```

In this examplehe the `strict=TRUE` parameter enables strict mode, treating any warning as an error and potentially causing the pipeline to exit if an issue occurs. Finally, the processed video is displayed using autovideosink.

## Plugin Parameters

The `insert_logo` plugin supports the following parameters:

- **alpha**: Set alpha (opacity) blending on the plugin. Range is 0 to 100.
- **coordinate**: X and Y coordinate value. Default is top right corner of the given resolution.
- **logo**: Path of logo file. If not provided, plugin will take default logo (Moschip logo).
- **rotation**: Rotate the logo. Options are NoRotate, Clockwise, AntiClockwise. Disabled when scrolling is enabled.
- **scroll**: Scroll side. Options are LeftToRight, RightToLeft, off.
- **speed**: Animation Speed. Options are slow, medium, fast.
- **strict**: Treat every warning as an error and exit the plugin when set to 'TRUE', else give warning and continue with default value.


Example Pipelines
1. Overlaying a logo on a video with default settings:
```console
 gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! insert_logo ! autovideosink
```
2. Overlaying a logo at a specific coordinate with rotation animation:
```console
 gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! insert_logo logo=/path/to/logo.png rotation=Clockwise speed=medium coordinate='<100,100>' ! autovideosink
```
3. Overlaying a logo with scrolling animation:
```console
 gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=640,height=480,framerate=24/1 ! insert_logo logo=/path/to/logo.png scroll=LeftToRight speed=slow coordinate='<200,200>' ! autovideosink
```
4. Overlaying a logo with alpha blending and strict mode enabled:
```console
 gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12,width=1280,height=720,framerate=30/1 ! insert_logo logo=/path/to/logo.png alpha=50 strict=TRUE ! autovideosink
```

## License
This project is licensed under the MIT License - see the LICENSE file for details.
GStreamer itself is licensed under the Lesser General Public License version 2.1 or (at your option) any later version: https://www.gnu.org/licenses/lgpl-2.1.html

## Contributing
Please read CONTRIBUTING.md for details on our code of conduct, and the process for submitting pull requests to us.

## Changelog
See the CHANGELOG.md file for details about the changes between versions.

## Acknowledgements
Hat tip to anyone whose code was used
Inspiration
etc
If you use this plugin in your project, consider adding your name/company to the acknowledgments.
