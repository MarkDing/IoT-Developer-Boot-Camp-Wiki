#!/bin/sh

copy_files() {
	# $1 -- dirname
		
	item_list=`ls $1`
	for item in $item_list
	do
		if [ ! -e files/$item ]; then
			#echo "create link files/$item --> ../$1/$item"
			#ln -s ../$1/$item files/$item
			mv -f $1/$item files/
		fi
	done
}

cd files
find ./ -type l | xargs rm -rf
cd ..

files_list=`find ./ -name 'files'`
#echo $files_list

for item in $files_list
do
	#echo $item
	if [ ! "$item" = "./files" ]; then
		copy_files $item
	fi
done


