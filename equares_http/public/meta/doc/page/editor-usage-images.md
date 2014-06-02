# Adding images

Images can be added to the [simulation info property](/doc#page/editor-usage-text-info). Use this syntax for images:<br/>
```![alt](url)```<br/>or<br/> ```![alt](url "title")```.

Also notice the **Image** button on the left from the **Ok** button in the [text editor](/doc#page/editorpane-text).
Clicking the **Image** button brings up the dialog for image selection or uploading.
The dialog contains two tabs (provided you are logged in).

## Picking existing images
The first tab allows you to select an image from the list of thumbnails. It also contains a filter you can use to find an image.

![](/meta/doc/page/editor-usage-images-1.png 'Image dialog (thumbnails)')

Clicking on an image thumbnail inserts the image into the property currently being edited and closes the dialog.

For images uploaded by you, the following can also be done:
* Click the ![edit](/images/edit.png) icon to modify the image (change title, keywords, or the image itself).
* Click the ![trash](/images/trash.png) icon to delete the image.

## Uploading images
The second tab allows you uploading new images:

![](/meta/doc/page/editor-usage-images-2.png 'Image dialog (upload)')

It is necessary to choose an image file and enter image title and keywords.
Keywords should be separated by comma; each keyword is allowed to be either a single word or a space separated list of words.
Once you click the **Upload** button, the image is uploaded to the server, a link to it is inserted to
the property currently being edited, and the dialog closes.

**Note:** Images uploaded to the server are identified by username and image name. The image name is the name of file you upload.
Specifying the name coinciding with the name of one of your previously uploaded images will cause server to reject the upload.
In this case you should rename the file being uploaded or remove the other image
(to replace the other image, edit it by pressing the ![edit](/images/edit.png) icon).
Notice also that when you provide a new file while editing an existing image, the name of the new file is ignored,
and the image retains the old name.
