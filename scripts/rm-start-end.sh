#! /bin/sh

#rm star-end symbols

sed 's/\(<s>\|<\/s>\)\+//g' | sed 's/^ \+//' | sed 's/ \+$//'

