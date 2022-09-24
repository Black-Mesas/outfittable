# outfittable
Simple tool to download ROBLOX outfits to file and equip them later.

## Basic Usage

Before doing anything, first go to `roblox.com` and open the dev console (right click and press Inspect, or press F11,
or ctrl+shift+I). From there, press the storage tab (in firefox). There should be a cookies tab in the tree somewhere in
there. Select the .roblox.com domain.

Copy your .ROBLOSECURITY and put it in a `cookie.txt` file in the same place as the .exe, without anything else in the
file. Make sure there are no new-lines or anything.

### Power user tip #1 
`outfittable.exe` will just read from the current working directory, so put the `cookie.txt` in your current working 
directory if you are going to use it from the command line.

(This also means some annoying things for the command line but I'll talk about that later.)

Double-click the .exe, and it should generate a `curr.fit` file and make a pop-up dialog announcing so. Rename this file as you see fit (pun intended.)
Once you have a few outfits saved, you can drag and drop one outfit file on the executable to set your current outfit on the site. More than 1 outfit however and it will ignore all of them and error.

(Power user tip: It just uses the first argument, and ignores all others. Unless of course, there is more than 1 argument. Then it ignores all arguments and goes into util processing mode.)

## Advanced usage

You can actually download other peoples avatars, however it requires some setup due to my laziness.

Using the command prompt, cd into the directory your cookie.txt is in (probably the same directory the .exe is in.)
As an example, you can download my avatar:

```shell
outfittable.exe -u 22456650
```

Just replace my user id with any user on the website's id, and it should download (you will still need your cookie.txt for this, sorry).
