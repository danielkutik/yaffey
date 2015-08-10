<h2>Yet Another Flash File (System) Editor YEAH!</h2>

<p>
A GUI utility for reading, editing and creating YAFFS2 images in Windows.<br>
Written in Qt 4.8.1. Links against official YAFFS code (included in source tree) for ECC algorithm.<br>
</p>

<b>Features</b>
<ul>
<li>Create new YAFFS2 image</li>
<li>Open existing image</li>
<li>Export files/directories from image</li>
<li>Import files and directories into image (permissions inherited from parent directory, dates created/modified/accessed set to current date & time)</li>
<li>Delete files</li>
<li>Edit filenames</li>
<li>Edit permissions</li>
<li>Edit user and group ids</li>
<li>Edit symbolic link aliases</li>
<li>ECC (Error Checking & Correction) support</li>
</ul>

<p><img src='http://dl.dropbox.com/u/8545722/yaffey/yaffey-v0.2-screenshot.png' /></p>

<b>New in v0.2</b>
<ul>
<li>Added import dialog to choose to import file(s) or a directory</li>
<li>Added support to import multiple files</li>
<li>Added support to import 1 directory at a time (using dialog)</li>
<li>Added drag & drop importing (which supports multiple directories & files!)</li>
<li>Added support to delete more than one item at a time</li>
<li>Added summary dialog after saving an image</li>
<li>Added text below icons in toolbar at the request of <b>varun.chitre15</b> @ XDA!</li>
</ul>

<b>Limitations</b>
<ul>
<li>Only handles YAFFS2 images with page size of 2112 (2048 chunk + 64 spare)</li>
<li>Only edit 1 file at a time (no bulk changing permissions)</li>
<li>Can't save changes to opened image, only save as...</li>
<li>Can't create directories (but can import directories!)</li>
<li>Can't create symbolic links</li>
<li>Can't export symbolic links</li>
<li>No support for hard links</li>
<li>No support for special files (device files, etc)</li>
<li>No option to disable ECC (always on)</li>
<li>UI synchronous with file operations so UI freezes when reading/writing</li>
</ul>

<p>
Flash images created with this utility to your device at your own risk.<br>
</p>