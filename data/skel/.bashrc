#
# ~/.bashrc
#

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias ls='ls --color=auto'

if   [[ "$XDG_VTNR" == "1"  ]]
then PS1="\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\n$ "
else PS1='[\u@\h \W]\$ '
fi

export LANG='en_US.UTF8'
setxkbmap us
