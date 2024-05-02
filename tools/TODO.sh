ag -rn "TODO: " . | awk -F ":" '{TODO=""; for (i=3; i<=NF; i++) {TODO=TODO$i} gsub("^ *", "", TODO); print $1":"$2"\t"TODO}' | grep -v "TODO.sh"
