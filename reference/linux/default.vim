"" {1   Some global functions, deprecated.
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"function! GetSystem()               " test the platform operation system
"    if (has("win32") || has("win95") || has("win64") || has("win16"))
"        return "windows"
"    elseif has("unix")
"        return "linux"
"    elseif has("mac")
"        return "mac"
"    endif
"endfunction
"
"let g:PlatformOS = GetSystem()
"" }1
"
"function ShortTabLabel ()
"    let bufnrlist = tabpagebuflist (v:lnum)
"    let label = bufname (bufnrlist[tabpagewinnr (v:lnum) -1])
"    let filename = fnamemodify (label, ':t')
"    return filename
"endfunction
"set guitablabel=%{ShortTabLabel()}

" {1   folder
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
if has('foldenable')
	set foldenable            " allow to fold
    finish
endif
nnoremap <space> za           " toggle folds using <space>
set foldcolumn=2              " indicate fold, + closed, - start of open fold | lines of fold
                              " NOTE: you can also click "+"/"-"/"|"
set foldopen&                 " set to the default, <set foldopen=all> will open folder when it
                              " under cursor.
"set foldclose=all             " folds close automatically when move out.
set foldlevel=0               " close all folds when indent
set foldmethod=manual         " syntax on MUST set, manual/indent/syntax/expr/diff
nmap <leader>vs :mkview <CR>
nmap <leader>vl :silent loadview <CR>
"autocmd BufWinLeave *.* mkview            " auto save/load view, some bugs with other plugins
"autocmd BufWinEnter *.* silent loadview
fu! CustomFoldText()          " custom the fold text.
    "get first non-blank line
    let fs = v:foldstart
    while getline(fs) =~ '^\s*$' | let fs = nextnonblank(fs + 1)
    endwhile
    if fs > v:foldend
        let line = getline(v:foldstart)
    else
        let line = substitute(getline(fs), '\t', repeat(' ', &tabstop), 'g')
    endif

    let w = winwidth(0) - &foldcolumn - (&number ? 8 : 0)
    let foldSize = 1 + v:foldend - v:foldstart
    let foldSizeStr = " " . foldSize . " lines "
    let foldLevelStr = repeat("+--", v:foldlevel)
    let lineCount = line("$")
    let foldPercentage = printf("[%.1f", (foldSize*1.0)/lineCount*100) . "%] "
    let expansionString = repeat(".", w - strwidth(foldSizeStr.line.foldLevelStr.foldPercentage))
    return line . expansionString . foldSizeStr . foldPercentage . foldLevelStr
endf
set foldtext=CustomFoldText()
if has("autocmd")
    autocmd FileType python setlocal fdm=indent foldignore=
endif
" }1

"" {1   misscellance
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
autocmd! bufwritepost .vimrc source %        "Automatic reloading of .vimrc
if has('mouse')               " in many terminal emulators the mouse works just fine, so enable it
  set mouse=nv                " Mouse only works in 'Normal' and 'Visual' Mode, a indicate all Mode.
endif
"autocmd BufEnter * cd %:p:h   " Change word dir to current dir
set autochdir                 " change directory, base on the opened file
set history=50	              " keep 50 lines of command line history
set confirm                   " when deal with unsaved of readonly file, a confirm window occur
set noerrorbells              " donot bells when error occurs
set vb t_vb=                  " no beeps
set hidden                    " Hide buffers when they are abandoned
set autowrite                 " Automatically save before commands like :next and :make
set autowriteall              " Autosave files
set wildmenu                  " show autocomplete menus
set wildignore=*.o,*~,*.pyc   " ignore compiled files
set showmode                  " how editing mode
set visualbell                " error bells are displayed visually
set backspace=eol,start,indent " Configure backspace so it acts as it should act
set whichwrap+=<,>,h,l         " Configure backspace so it acts as it should act
set cmdheight=2               " height of the command bar
set nobackup                  " Disable stupid backup adn swap files - they trigger too
set nowritebackup             " many for file system watchers
set noswapfile
set autoread                  " set to auto read when a file is changed from the outside
let mapleader=','             " set <mapleader> for some plugins
let g:mapleader=','
set lazyredraw                " Don't redraw while executing macros (good performance config)
set magic                     " For regular expressions turn magic on

" No annoying sound on errors
set noerrorbells
set novisualbell
set t_vb=
set tm=500

" use modeline
set modeline

" Treat long lines as break lines (useful when moving around in them)
map j gj
map k gk
" }1

" {1   indent, space, TAB relative -- details in *HTML*
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"set autoindent              " the current line's indent level to set the indent level of new lines)
set smartindent             " better than auto indent OR <set autoindent>
set softtabstop=4           " mix method with TAB and space
set shiftwidth=4            " width when auto indent
set tabstop=4               " the width we TAB enter
set expandtab               " substitute the TAB with space, OR <set noexpandtab>
set listchars=tab:▸\ ,trail:-,extends:>,precedes:<,eol:¬    " howto display invisual char
set nolist                  " do NOT display TAB(^I) and EOL($), OR <set list>
" }1

" {1   search/match options
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set hlsearch                " hilight the key words, OR <set nohlsearch>
set incsearch               " do incremental searching
set showmatch               " show matching brackets (),{},[]
set matchtime=2             " show matching brackets for 1 seconds
set ignorecase              " ignore cases when search
set smartcase               " /step will match "Step", "Steph", "stepbroth", "misstep.
                            " /Step will match "Step", "Steph", but not "stepbroth" or "misstep."
noremap <C-n> :nohl<CR>     " removes highlight of your last search by Ctrl-n
vnoremap <C-n> :nohl<CR>
inoremap <C-n> :nohl<CR>
" }1

" {1   file type auto detect -- details in *HTML*
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
filetype on                 " detect filetype automaticly
filetype plugin on          " load the relative plugins according to filetype
filetype indent on          " load indent files for specific filetype
""EQUAL TO"""<filetype plugin indent on>
" }1

" {1   tabs
"next"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set showtabline=2   " 0 never display tab, 1 default, 2 always display.
set tabpagemax=18   " max tabs, default 10.
" easier moving between tabs
map <leader>p <esc>:tabprevious<CR>
map <leader>n <esc>:tabnext<CR>
map <leader>e <esc>:tablast<CR>
map <leader>f <esc>:tabfirst<CR>
map <leader>c <esc>:tabclose<CR>
"map <S-Left> :tabp<CR>    " Shift+Left
"map <S-Right> :tabn<CR>    " Shift+Right

" Useful mappings for managing tabs
map <leader>tn :tabnew<cr>
map <leader>to :tabonly<cr>
map <leader>tm :tabmove

" Opens a new tab with the current buffer's path
" Super useful when editing files in the same directory
map <leader>te :tabedit <c-r>=expand("%:p:h")<cr>/

" Switch CWD to the directory of the open buffer
map <leader>cd :cd %:p:h<cr>:pwd<cr>

" Specify the behavior when switching between buffers
try
    set switchbuf=useopen,usetab,newtab
    set stal=2
catch
endtry
" Return to last edit position when opening files (You want this!)
autocmd BufReadPost *
    \ if line("'\"") > 0 && line("'\"") <= line("$") |
    \   exe "normal! g`\"" |
    \ endif
" Remember info about open buffers on close
set viminfo^=%
" }1

" {1   buffers
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
map <leader>b <esc>:buffers<CR>
map <leader>bn <esc>:bnext<CR>
map <leader>bp <esc>:bprevious<CR>
set hid                          " A buffer becomes hidden when it is abandoned
map <leader>bd :Bclose<cr>       " Close the current buffer
map <leader>ba :1,1000 bd!<cr>   " Close all the buffers
" }1

" {1   windows
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" bind Ctrl+<movement> keys to move around the windows, instead of using Ctrl+w+<movement>.
map <c-h> <c-w>h
map <c-j> <c-w>j
map <c-k> <c-w>k
map <c-l> <c-w>l
map <c-a> <c-w>_
imap <a-h> <esc><c-w>hi
nmap <leader>k :resize +6<CR>             " resize windows through w=/w-/w,/w.
nmap <leader>j :resize -6<CR>
nmap <leader>h :vertical resize -6<CR>
nmap <leader>l :vertical resize +6<CR>
" }1

" {1   syntax/highlight setting
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
syntax on                   " enable syntax highlighting. OR <syntax on>
if has("gui_running")       " load my color scheme, ron, solarized, slate, evening, desert, morning, pablo, torte, darkblue, etc.
    set background=light
    colorscheme desert      " you can type :colorscheme <Tab> to search the supporting scheme.
    set guioptions-=T
    set guioptions+=e
    set guitablabel=%M\ %t
    set t_Co=256
else                        " Or
    " blue darkblue delek desert koehler solarized elflord(*)
    set background=dark         " dark background
    colorscheme solarized
    let g:solarized_termcolors=256
endif
" }1

" {1   encoding
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set encoding=utf8          " Set utf8 as standard encoding and en_US as the standard language
set ffs=unix,dos,mac       " Use Unix as the standard file type
" }1

" {1   display
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set number                    " display the line number
set scrolloff=3               " keep the cursor 3 lines from top or bottom
set showcmd                   " display incomplete commands.
set cursorline                " highlight current line.
set cursorcolumn              " highlight current column.
"set lbr                       " Linebreak on 100 characters
"set textwidth=100             " set the width of text.
"set colorcolumn=+1            " NOTE: if 'mkview' used the line may not display automatically
set wrap                      " display chars in one line, even longer than textwidth
highlight colorcolumn ctermbg=235
set ruler                     " status bar and ruller for each windows
"set rulerformat=%-14.(%l,%c%V%)\ %P
" NOTE:bold/underline/reverse/italic/standout can used for 'cterm' option.
hi CursorLine   cterm=reverse ctermbg=NONE ctermfg=NONE guibg=NONE guifg=NONE
hi CursorColumn cterm=NONE ctermbg=gray ctermfg=NONE guibg=black guifg=NONE
" details see: http://vim.wikia.com/wiki/Remove_unwanted_spaces
highlight WhitespaceEOL ctermbg=red guibg=red            " highlight the white space in EOL
match WhitespaceEOL /\s\+$/
"autocmd BufWritePre * :%s/\s\+$//e   " replace whitespace when write to file.
" }1

" {1   grep-related
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"set grepprg=grep\ -nH   " grep -n $* /dev/null(default), grep command.
"set grepformat=        " %f:%l%m,%f  %l%m(default), output format.
" }1

" vim: foldmarker={,} foldlevel=0 foldmethod=marker :
