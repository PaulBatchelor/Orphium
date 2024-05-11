if [ ! -d blocks ]
then
    tar xvf blocks.tar.gz
fi

./tools/block-import
./tools/insert_block 0 newblocks/0.txt
