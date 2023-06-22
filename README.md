# mpcd: modular performant clone detector

Read from the standard input tokenized files and output
sets of clones.

## Build

```
cd src
make
```


## Install

```
cd src
sudo make install
```

## Run

The clone detector's input is a stream of file identifiers
(e.g. file paths) prefixed with `F`, followed by each file's
tokens for each line in the range 0–699.
These can be obtained by applying the
[tokenizer](https://github.com/dspinellis/tokenizer) program on each file.

### Example

The following example identifies clones for all Java files
located in the `src` directory.

```sh
find src -type f -name '*.java' -print0 |
while IFS= read -r -d '' file; do
  echo "F$file"
  tokenizer -l Jave -o line -t N "$file"
done |
mpcd >results.txt
```

## Reference manual
You can read the command's Unix manual page through [this link](https://dspinellis.github.io/manview/?src=https%3A%2F%2Fraw.githubusercontent.com%2Fdspinellis%2Fmpcd%2Fmaster%2Fsrc%2Fmpcd.1&name=mpcd(1)&link=https%3A%2F%2Fgithub.com%2Fdspinellis%2mpcd).

