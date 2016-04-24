#!/bin/bash

if(($1 == "diff"))
then
git difftool --tool=vimdiff --no-prompt
fi

