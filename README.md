# outfittable
Simple tool to download ROBLOX outfits to file and equip them later.

## Usage

```
Usage: outfittable [OPTION] FILE
Downloads Roblox avatars to file.
  -u, --user=USERID     User id of avatar to download
  -c, --cookie=FILE     Cookie file location
  -o, --output=FILE     Output file location
  -h, --help            Shows command help
```

If `FILE` is defined, reads avatar data from the filepath specified, and sets it using the cookie.

If explicitly specified, `--user` does not require `--cookie` to be defined. Else, it defaults to the userid of the  cookie.

`--cookie` defaults to `./cookie.txt` (in cwd).

`--output` defaults to `./curr.fit` (also in cwd).