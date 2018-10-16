#!/usr/bin/env bash
# Usage: ./mk <dir> <make target>
# No relation to mk(1)

if (( $# < 1)); then
	echo "Specify a project to build."
	exit 1
fi

projects="cntl interface libmca66 mca66d"

# Build sub-project
make_project () {
	dir=$1
	targ=$2

	echo "== Processing:" $dir $targ " =="
	if [ -d "$dir" ]; then
		cd $dir
		make $targ
		cd ..

	else
		echo Invalid build directive $dir.

	fi
}


dir=$1
targ=$2
all="false"

# Process all or cleanall
if [ "$dir" = "all" ]; then
	# Build everything
	targ="build"
	dir=""
	all="true"

elif [ "$dir" = "cleanall" ]; then
	# Clean everything
	targ="clean"
	dir=""
	all="true"

# Shortcuts
elif [ "$dir" = "lib" ]; then
	dir="libmca66"

elif [ "$dir" = "daemon" ]; then
	dir="mca66d"

else
	targ=$2

fi

# Perform sub-project actions
if [ "$all" = "true" ]; then
	for dir in $projects
	do
		make_project $dir $targ
	done

else
	# Default to building sub-project
	make_project $dir $targ

fi
