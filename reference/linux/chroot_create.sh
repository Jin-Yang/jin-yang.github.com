#!/bin/bash

########### FOR USER BEGIN ###################
# define user commands.
cmdlist="/usr/bin/ps "
cmdlist+="/usr/bin/mount "

# some specific steps needed when create chroot environment.
user_process() {
	true
}

# some specific steps needed before enter chroot.
before_enter_jail() {
	true
}
########### FOR USER END #####################


# Create chroot jail, chroot directory set as ${PWD}/root.
chroot_path="`pwd`/root"
# Display details.
verbose=no
# Call chroot command directly.
create=no

if [ ! `id -u` -eq 0 ];then
    echo "Change user to 'root', and try again"
    exit 1
fi

# Usage: usage, Print the usage.
usage () {
cat <<EOF
Usage: chroot_create [OPTION] [PATH]
Create a basic chroot environment based on your host Linux.

   -h, --help           printf this message and exit
   -c, --create         create the chroot environment before call 'chroot' command
   --root=DIR or DIR    the directory want to change, default root/
   --verbose            display the details
EOF
}

argument () {
	opt="$1"
	shift

	if test $# -eq 0; then
		echo "$0: option requires an argument -- '$opt'" 1>&2
		exit 1
	fi
	echo "$1"
}

# Check the arguments.
while test $# -gt 0
do
    option=$1
    shift

    case "$option" in
    -h | --help)    # help
        usage
        exit 0 ;;

    --root)         # the argument is "--root dir"
        chroot_path=`argument $option "$@"`; shift;;
    --root=*)       # the argument is "--root=dir"
        chroot_path=`echo "$option" | sed 's/--root=//'`;;

    -v | --verbose)
        verbose=yes ;;

    -c | --create)
        create=yes ;;

    -* )
        echo "Unrecognized option $option" 1>&2
        echo 1>&2
        usage
        exit 1
        ;;
    *)
        chroot_path="$option"
        ;;
    esac
done

process () {
	echo "Chroot jail(PATH=${chroot_path}) setting begins."
    if [ ! -d ${chroot_path} ]; then
        mkdir -p ${chroot_path}
    fi

	####### 1 The basic directory list want to create.
	dirlist="$chroot_path "
    dirlist+="$chroot_path/lib "
    dirlist+="$chroot_path/lib64 "
    dirlist+="$chroot_path/usr/lib64 "
    dirlist+="$chroot_path/bin "
	dirlist+="$chroot_path/sbin "
    dirlist+="$chroot_path/usr/bin "
    dirlist+="$chroot_path/etc "
    dirlist+="$chroot_path/sys "
    dirlist+="$chroot_path/proc "
	dirlist+="$chroot_path/dev "
	dirlist+="$chroot_path/dev/pts "
    dirlist+="$chroot_path/home "
    dirlist+="$chroot_path/root "

	# Create them or use "/bin/mkdir -p $chroot_path/{etc,lib} " if you like.
	echo &&
	echo "[1]  Creating directory....." &&
	for dir in $dirlist; do
		/bin/mkdir -p $dir
		if [ x"$verbose" == xyes ]; then
			echo "/bin/mkdir -p $dir"
		fi
	done &&
	echo " +++++++++++++++++++++++++++++ [OK]"


	####### 2 The commands you want to add to chroot jail.
	cmdlist_min="/bin/bash /usr/bin/env"
	cmdlist_def="/bin/ln /bin/cat /bin/ls /bin/mv /bin/cp /bin/rm /bin/mkdir "
	cmdlist_def+="/bin/rmdir /bin/pwd /usr/bin/whoami "

	cmdlist_all="$cmdlist_min $cmdlist_def $cmdlist"

	# Copy the commands to destination directory.
	echo &&
	echo "[2]  Copy commands ........." &&
	# The commands we defined do not have a symbolic link.
	for i in ${cmdlist_min} ${cmdlist_def}; do
		cp -p $i $chroot_path$i
		if [ x"$verbose" == xyes ]; then
			echo "cp -a $i $chroot_path$i"
		fi
	done &&
	# The user defined commands may have symbolic link.
	for i in ${cmdlist}; do
		cp -a $i $chroot_path$i
		if [ x"$verbose" == xyes ]; then
			echo "cp -a $i $chroot_path$i"
		fi

		# This is a symblic link, so only the symbolic link has copied.
		if [ -h $i ]; then
			real=`ls -l $i | awk '{ print $NF }'` # get the real file.
			if [ "${real:0:1}" = "/" ]; then
				# the symbolic file is a absolute file.
				cp -af $real ${chroot_path}${real}
			else
				dir=${i%/*}    # get the directory of the real file.
				cp -af $dir/$real ${chroot_path}$dir/$real
			fi
		fi
	done &&
	echo " +++++++++++++++++++++++++++++ [OK]"



	####### 3 Copy the libs to destination directory.
	# Get the corresponding libs.
	libs=`ldd $cmdlist_all | awk '{ print $1 "\n" $3 }' | grep -E "^/lib" | sort | uniq`

	echo &&
	echo "[3]  Copy libs ............" &&
	for i in ${libs}; do
		cp -a $i ${chroot_path}$i
		if [ x"$verbose" == xyes ]; then
			echo "cp -a $i ${chroot_path}$i"
		fi

		# This is a symblic link, so only the symbolic link has copied.
		if [ -h $i ]; then
			real=`ls -l $i | awk '{ print $NF }'` # get the real file.
			if [ "${real:0:1}" = "/" ]; then
				# the symbolic file is a absolute file.
				cp -f $real ${chroot_path}${real}
			else
				dir=${i%/*} # get the directory of the real file.
				cp -f $dir/$real ${chroot_path}$dir/$real
			fi
		fi
	done &&
	echo " +++++++++++++++++++++++++++++ [OK]"


	####### 4 Set something else.
	echo
	echo "[4]  Set something else....."

	if [ ! -c "$chroot_path"/dev/console ]; then
		/bin/mknod -m 600 $chroot_path/dev/console c 5 1
		if [ x"$verbose" == xyes ]; then
			echo "/bin/mknod -m 600 $chroot_path/dev/console c 5 1"
		fi
	fi

	if [ ! -c "$chroot_path"/dev/null ]; then
		/bin/mknod -m 600 $chroot_path/dev/null c 1 3
		if [ x"$verbose" == xyes ]; then
			echo "/bin/mknod -m 600 $chroot_path/dev/null c 1 3"
		fi
	fi
	if [ ! -h "$chroot_path/bin/sh" ]; then
		cd $chroot_path/bin && ln -s bash sh && cd - 1>/dev/null
	fi

	echo "root:x:0:0:root:/root:/bin/sh" > $chroot_path/etc/passwd
	echo "root:*:15821:0:99999:7:::" > $chroot_path/etc/shadow
	echo "root:x:0:" > $chroot_path/etc/group

	/bin/chmod 0644 $chroot_path/etc/{passwd,shadow,group}
	/bin/chown root:root $chroot_path/etc/{passwd,shadow,group}


	# NOTE: Though /etc/passwd has been added, we also get "I have no name!"
	#       problem. To solve this the following should be done.
	echo -e "passwd:    files\nshadow:    files" > ${chroot_path}/etc/nsswitch.conf
	/bin/chmod 0644 $chroot_path/etc/nsswitch.conf
	/bin/chown root:root $chroot_path/etc/nsswitch.conf

    for p in "/usr/lib* /lib"; do
	   libnss=`find $p -name "libnss_files.so.2"`
       if [ x"$libnss" != x ]; then
           cp $libnss ${chroot_path}/lib/
       fi
    done

	echo "PS1='\u:\w\$ '" > $chroot_path/etc/profile &&
	echo "alias ls='ls --color=tty'" >> $chroot_path/etc/profile

	if [ x"$verbose" == xyes ]; then
		echo "add /etc/passwd"
		echo "add /etc/nsswitch.conf"
		echo "add /etc/profile"
	fi
	echo " +++++++++++++++++++++++++++++ [OK]"
}


if [ x"$create" == xyes ]; then
	process
    user_process
fi

before_enter_jail

echo &&
echo "Changing to the directory."
## env -i: clear all variables of the chroot environment.
## TERM="$TERM": Set the TERM variable inside chroot to the same valued as outside chroot.
## +h: hashing is switched off by passing the +h option to bash.
/usr/sbin/chroot "$chroot_path" /usr/bin/env -i \
			HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
			PATH=/bin:/sbin:/usr/bin \
		   	LD_LIBRARY_PATH=/lib \
			/bin/bash --login +h
