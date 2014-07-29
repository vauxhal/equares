# Using snippets

When editing a property declaring a **snippet type** (see =[Snippets](/doc#page/general-snippets)), the =[text editor](/doc#page/editorpane-text) displays
the **Snippet** button. Click it to bring up the **Snippet edior** dialog. The dialog will open and you will see the list of available snippets:

![](/meta/doc/page/editor-usage-snippets-1.png 'Snippet editor dialog')

The dialog contains the list of available snippets on the left. The list can be filtered by using the search bar at the top of the dialog.

Click a snippet title in the list to make it current. You will then see the documentation for the snippet on the right. The title of the current
snippet is underlined so user can understand it is the current one.

## Picking existing snippet

1. Find the snippet you are going to pick and click its title in the list of snippet.
2. Click the **Pick** button at the bottom of the dialog.

## Creating new snippet

1. You might want to start from scratch or base your snopped on top of an existing one.
   * To create new snippet from scratch, click the **New snippet** button at top right corner of the dialog.
   * To create new snippet based on an existing one, find that snippet and make it current. Then click the **Clone snippet** button
     on the left of snippet documentation.
   In either case, the snippet source will appear on the left of the dialog in place of the list:<br/>
   ![](/meta/doc/page/editor-usage-snippets-2.png 'Editing snippet code')
2. Enter snippet title twice: first, at the beginning of the documentation block, in line starting with ```title: ```; also add title
   as part of documentation page, after heading mark ```#```.
3. Enter snippet keywords in the documentation block in line starting with ```keywords: ```. This will help to find the snippet by keywords.
   Notice that a keyword can be a single word or several words separated by space; keywords are separated by comma.
4. Enter snippet documentation and source code.
5. Click the **Save snippet** button.

## Editing existing snippet
1. Find snippet you want to edit and make it current. Then click the **Edit snippet** button
   on the left of snippet documentation.
   * You only can edit your own snippets. To edit someone else's snippet, clone it.
2. Edit snippet documentation and/or code.
3. Click the **Save snippet** button.

## Important notes
* Snippet documentation goes in block starting with line ```/*#``` and ending with line ```*/```.
* Snippet documentation is displayed in the =[info pane](/doc#page/editorpane-info) when the box having one property with snippet is selected,
  or when a property with snippet is being edited.
* You can use ?[markdown text](http://daringfireball.net/projects/markdown/) (or ?[gfm](https://help.github.com/articles/github-flavored-markdown)),
  as well as ?[TeX](http://tug.org/) formulas. For more information, see =[simulation info property](/doc#page/editor-usage-text-info)
* Snippets have names based on their titles. An attempt to save snippet with the same title as one of your other snippets will fail. In this case,
  change snippet title. Notice also that the existing snippet with the same title as new snippet will never get overwritten.
