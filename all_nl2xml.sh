 
#!/bin/bash

# loop through files
for file in "$@"; do
  # test to see if it's a regular file
  if [[ -f $file ]]; then
    name=${file#*/}
    name=${name%.*}
    #replace spaces
    name=${name//[ ]/}
    ./nl2xml $file > musicxml/$name.xml
  fi
done
