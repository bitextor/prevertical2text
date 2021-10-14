# prevertical2text
Extracts plain text, language identification and more metadata from Spiderling prevertical files

## Download
Clone this repo along with submodules:
```
git clone --recurse-submodules https://github.com/bitextor/prevertical2text.git
```
Or:
```
git clone https://github.com/bitextor/prevertical2text.git
git submodule update --init
```

## Compile
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/your/prefix/path ..
# cmake .. -DCMAKE_BUILD_TYPE=Debug # for debug
make -j
make install
```

## Usage
```
prevertical2text -o <output_folder> [ -f <output_files> ] <prevertical_file>...
```
* `--output`/`-o` output folder
* `--files`/`-f` list of output files separated by commas (and without `.gz`); `text` and `url` are always written, while `mime` and `html` are optional
* `--url-filters` file containing regular expressions that match urls of documents to eliminate
* `--verbose`/`-v` print progress and filtering information
* `--silent`/`-s` print only warnings and errors

  URL Filter format is a single regular expression per line.

  Lines beginning with `#` and empty lines are ignored. Any invalid filter will raise a warning message, but will not prevent other filters from being read.

## Included dependencies
HTML Tokenizer by [c-smile](https://www.codeproject.com/Articles/14076/Fast-and-Compact-HTML-XML-Scanner-Tokenizer)

HTML entities decoder by [Christoph Gärtner](https://bitbucket.org/cggaertner/cstuff/src/master/entities.c)
___

![Connecting Europe Facility](https://www.paracrawl.eu/images/logo_en_cef273x39.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
