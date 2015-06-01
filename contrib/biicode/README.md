# Biicode Integration
MeTA can be built using biicode. To do so, symlink the biicode.conf file to
the project's root directory like so (from the project root):

```bash
ln -s contrib/biicode/biicode.conf .
```

Then, configure and build with the following:

```bash
bii init -L
bii configure
bii build
```

After placing the sample config.toml in the `bin/` directory, you can run
the unit tests:

```bash
bii test
```
