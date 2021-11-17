# Notions of Time

## Build and run

As long an up-to-date version of Haskell's stack is installed do:

```
stack ghci
```

then at run something like:

```haskell
*Main Lib> parseString "bd sd bd [sd sd]"
```

which will output:

```haskell
PSeq [PT (TStr "bd"),PT (TStr "sd"),PT (TStr "bd"),PSeq [PT (TStr "sd"),PT (TStr "sd")]]
```

We can "compile" this with the command:

```haskell
*Main Lib> subdiv $ parseString "bd sd bd [sd sd]"
```

which will output:

```haskell
[PPLit "bd",PPLit "-",PPLit "sd",PPLit "-",PPLit "bd",PPLit "-",PPLit "sd",PPLit "sd"]
```

We can create a flatten output with:

```haskell
flatten' $ subdiv $ parseString "bd sd bd [sd sd]"
```

which will output:

```haskell
"bd - sd - bd - sd sd"
```

All this is combined in to a single function:

```haskell
*Main Lib Paths_pat> flatten "bd - bd - |:| hh - sn"
``` 

Here using a polyrhythmic merge and will output:

```haskell
"[bd hh]  [- -]  [- -]  [- -]  [- -]  [- -]  [bd -]  [- -]  [- sn]  [- -]  [- -]  [- -] "
```

> note that this is a fully expanded sequence and "notes" within [] represents notes 
> that are played concurrently with on another.
# pat
