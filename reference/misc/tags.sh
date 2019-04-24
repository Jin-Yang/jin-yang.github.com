#!/bin/sh
# Generate tags or cscope files
# Usage tags.sh <mode>
#
# mode may be any of: tags, TAGS, cscope
# README: ./tags.sh tags

tree=
# RCS_FIND_IGNORE has escaped ()s -- remove them.
ignore="$(echo "$RCS_FIND_IGNORE" | sed 's|\\||g' )"
# tags and cscope files should also ignore MODVERSION *.mod.c files
ignore="$ignore ( -name *.mod.c ) -prune -o"
# RHEL tags and cscope should also ignore redhat/rpm
ignore="$ignore ( -path redhat/rpm ) -prune -o"

DIR_LIST='storage client sql sql-common mysys vio libmysql libmysqld plugin'

# find sources in arch/$ARCH
find_arch_sources()
{
	for i in $archincludedir; do
		prune="$prune -wholename $i -prune -o"
	done
	find ${tree}arch/$1 $ignore $subarchprune $prune -name "$2" -print;
}


find_sources()
{
	find_arch_sources $1 "$2"
}

all_sources()
{
	find ${tree}include $ignore -name config -prune -o -name "*.[chS]" -print;

	for dir in $DIR_LIST; do
        find ${tree}$dir $ignore \( -name "*.[ch]" -o -name "*.cc" \) -print;
	done

#	find ${tree}* $ignore \
#	     \( -name include -o -name arch -o -name '.tmp_*' \) -prune -o \
#	       -name "*.[chS]" -print;
}


docscope()
{
	(echo \-k; echo \-q; all_target_sources) > cscope.files
	cscope -b -f cscope.out
}

all_target_sources()
{
    all_sources | grep -v 'test'
}

exuberant()
{
	all_target_sources | xargs $1 -a                        \
	-I __initdata,__exitdata,__initconst,__devinitdata	\
	-I DEFINE_TRACE,EXPORT_TRACEPOINT_SYMBOL,EXPORT_TRACEPOINT_SYMBOL_GPL \
	-I static,const						\
	--extra=+f --c++-kinds=+x                                \
	--regex-asm='/^(ENTRY|_GLOBAL)\(([^)]*)\).*/\2/'        \
	--regex-c='/^SYSCALL_DEFINE[[:digit:]]?\(([^,)]*).*/sys_\1/' \
	--regex-c++='/^TRACE_EVENT\(([^,)]*).*/trace_\1/'		\
	--regex-c++='/^DEFINE_EVENT\([^,)]*, *([^,)]*).*/trace_\1/'	\
	--regex-c++='/_PE\(([^,)]*).*/PEVENT_ERRNO__\1/'		\
	--regex-c='/PCI_OP_READ\((\w*).*[1-4]\)/pci_bus_read_config_\1/' \
	--regex-c='/(^\s)OFFSET\((\w*)/\2/v/'				\
	--regex-c='/(^\s)DEFINE\((\w*)/\2/v/'
}

case "$1" in
	"cscope")
		docscope
		;;

	"tags")
		rm -f tags
		exuberant ctags
		remove_structs=y
		;;

	"src")
		all_target_sources
		;;
esac

