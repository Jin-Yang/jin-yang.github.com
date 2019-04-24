" {1   modelines setting
" vim: foldmarker={,} foldlevel=0 foldmethod=marker :
" Abort if running in vi-compatible mode or the user doesn't want us.
"if &cp || exists('g:tabular_loaded')
"    echo "Not loading Tabular in compatible mode."
"    finish
"endif
"echo "Not loading Tabular in compatible mode."
"if &compatible
"    finish
"endif
"if has("compatible")
"    finish
"endif
"if version >= 700
"  finish
"endif
" }1

if version >= 700

" {1   plugin: Vundle(Mange plugins)
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" Use Vim settings, rather than Vi settings (much better!).
" This must be first, because it changes other options as a side effect.
set nocompatible
filetype off                   " required!
set rtp+=~/.vim/bundle/vundle/ " add directory to runtimepath
call vundle#rc()
" let Vundle manage Vundle self.
Bundle 'gmarik/vundle'
"""""""""""""""""""""""""""""""          My Bundles here         """""""""""""""""""""""""""""""""
"" original repos on github

"" ultimate solution for snippets, SirVer/ultisnips
Bundle 'UltiSnips'
"" snippets files for various programming languages
Plugin 'honza/vim-snippets'
" auto align tools.
Bundle 'godlygeek/tabular'

"<<<<<<<<<<<< naviagtion & search >>>>>>>>>>>>>>
" show tags, better than taglist.
Bundle 'majutsushi/tagbar'
" finder with regexp support
Bundle 'kien/ctrlp.vim'
" display the directory.
Bundle 'scrooloose/nerdtree'
Bundle 'jistr/vim-nerdtree-tabs'

"<<<<<<<<<<<<<<<<<<<< misc >>>>>>>>>>>>>>>>>>>>>
" solarized colorscheme
Bundle 'altercation/vim-colors-solarized'
" check color number.
Bundle 'guns/xterm-color-table.vim'
" a great start screen, list some info like recent open...
Bundle 'mhinz/vim-startify'
" a better support for line number.
Bundle 'myusuf3/numbers.vim'
" similar with vim-powerline but smaller.
Bundle 'vim-airline/vim-airline'
Plugin 'vim-airline/vim-airline-themes'
"Bundle 'Lokaltog/vim-powerline'

"<<<<<<<<<<<<<<<<< language >>>>>>>>>>>>>>>>>>>>
" syntax checking plugin for vim
Bundle 'scrooloose/syntastic'
" neo-completion with cache, need +lua(vim --version).
Bundle 'Shougo/neocomplete.vim'
"
Bundle 'Valloric/YouCompleteMe'


"Bundle 'klen/python-mode'




"" Go (golang) support for Vim
Bundle 'fatih/vim-go'
"
"" support for Chinese.
"Bundle 'mbbill/fencview'
"
"
"
"" miscellease about quickfix list/location list.
""Bundle 'milkypostman/vim-togglelist'
"
"
"
"
"Bundle 'Shougo/neosnippet'
"
"Bundle 'scrooloose/nerdcommenter'
""Bundle 'wesleyche/Trinity'
"Bundle 'wesleyche/SrcExpl'
""Bundle 'sjl/gundo.vim'
"Bundle 'Lokaltog/vim-easymotion'

"Bundle 'jlanzarotta/bufexplorer'
" git
"Bundle 'tpope/vim-fugitive'
"YCM will handle the syntasitic automatically.
"Bundle 'kevinw/pyflakes-vim'
" for wiki
"Bundle 'tomtom/viki_vim'
"" vim-scripts repos, use name directly
" needed by viki_vim
"Bundle 'tlib'
" needed by lookupfile
"Bundle 'genutils'
"Bundle 'lookupfile'
"vim中的杀手级插件: EasyGrep
"Bundle 'EasyGrep'
"Bundle 'grep.vim'

"Bundle 'Visual-Mark'

"Bundle 'winmanager'
" python colour scheme
"Bundle 'L9'
"" non github repos
"Bundle 'git://git.wincent.com/command-t.git'
"" git repos on your local machine (ie. when working on your own plugin)
"Bundle 'file:///Users/gmarik/path/to/plugin'


"pyclewn  "debug for vim.
"Conque-Shell    " use shell in vim
" }1

source ~/.vim/defaults.vim

" {1   plugin: tagbar  F3
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"let g:loaded_tagbar=0                   " do NOT load plugin, unless commented.
nmap <silent> <F3> :TagbarToggle<CR>
nmap <leader>bp :TagbarTogglePause<CR>
set updatetime=100
let g:tagbar_ctags_bin = 'ctags'
let g:tagbar_width = 40                 " set the width
let g:tagbar_autofocus=0                " jump to main windows.
let g:tabular_autoclose=0               " always display the win.
let g:tagbar_sort = 0                   " 关闭排序，按标签本身在文件中的位置排序
"let g:tagbar_autopreview = 1            " 开启自动预览，随着光标在标签上的移动，顶部会出现一个实时的预览窗口
let g:tagbar_foldlevel = 2
"let g:tagbar_right = 1                  " on the right.
let g:tagbar_left = 1                  " on the left.
autocmd FileType c,cpp nested :TagbarOpen
"autocmd BufReadPost *.cpp,*.c,*.h,*.hpp,*.cc,*.cxx call tagbar#autoopen()
"autocmd VimEnter * nested :call tagbar#autoopen(1)    " auto-open
" }1

" {1   plugin: Tabular   <leader>a=
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"let g:tabular_loaded = 1    " avoid loading tabular
if exists(":Tabularize")
    nmap <Leader>a= :Tabularize /=<CR>
    vmap <Leader>a= :Tabularize /=<CR>
    nmap <Leader>A= :Tabularize /=\zs<CR>
    vmap <Leader>A= :Tabularize /=\zs<CR>
    nmap <Leader>a: :Tabularize /:<CR>
    vmap <Leader>a: :Tabularize /:<CR>
    nmap <Leader>A: :Tabularize /:\zs<CR>
    vmap <Leader>A: :Tabularize /:\zs<CR>
    nmap <Leader>a, :Tabularize /,<CR>
    vmap <Leader>a, :Tabularize /,<CR>
    nmap <Leader>A, :Tabularize /,\zs<CR>
    vmap <Leader>A, :Tabularize /,\zs<CR>
    nmap <Leader>a<Bar> :Tabularize /<Bar><CR>
    vmap <Leader>a<Bar> :Tabularize /<Bar><CR>
    nmap <Leader>A<Bar> :Tabularize /<Bar>\zs<CR>
    vmap <Leader>A<Bar> :Tabularize /<Bar>\zs<CR>
endif
inoremap <silent> <Bar>   <Bar><Esc>:call <SID>align()<CR>a
function! s:align()
    let p = '^\s*|\s.*\s|\s*$'
    if exists(':Tabularize') && getline('.') =~# '^\s*|' && (getline(line('.')-1) =~# p || getline(line('.')+1) =~# p)
        let column = strlen(substitute(getline('.')[0:col('.')],'[^|]','','g'))
        let position = strlen(matchstr(getline('.')[0:col('.')],'.*|\s*\zs.*'))
        Tabularize/|/l1
        normal! 0
        call search(repeat('[^|]*|',column).'\s\{-\}'.repeat('.',position),'ce',line('.'))
    endif
endfunction
" }1

" {1   plugin: CtrlP
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" do NOT load plugin, unless commented.
"let g:loaded_ctrlp=1
" }1

" {1   plugin: Solarized Colorscheme
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" details check "syntax/highlight setting"
" }1

" {1   plugin: NERDTree(NERDTreeTabs used)  F4
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" do NOT load plugin, unless commented.
"let loaded_nerd_tree=0

" map F3 to toggle nerdtree window.
"map <F3> :NERDTreeTabsToggle<CR>
" auto quit when only nerdtree window.
"autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTreeType") && b:NERDTreeType == "primary") | q | endif
"autocmd vimenter * NERDTree    " open nerdtree window automatically.
let NERDTreeShowBookmarks=1    " display bookmark automatically.
let NERDTreeStatusline=''      " set the status line.
let NERDTreeWinPos="right"     " put nerdtree win to right, default left.
"let g:NERDTreeWinSize=20       " Set the window's width
"""""""""""""""""""""""""""              NERDTreeTabs            """""""""""""""""""""""""""""""""""
map <F4> :NERDTreeTabsToggle<CR>
" open NERDTree on gvim/macvim startup, default 1
let g:nerdtree_tabs_open_on_gui_startup=0
" open NERDTree on console vim startup, default 0
let g:nerdtree_tabs_open_on_console_startup=0
" do not open NERDTree if vim starts in diff mode, default 1
let g:nerdtree_tabs_no_startup_for_diff=0
" on startup, focus NERDTree if opening a directory, focus file if opening a file.
" (When set to `2`, always focus file window after startup), default 1.
let g:nerdtree_tabs_smart_startup_focus=2
" open NERDTree on new tab creation (if NERDTree was globally opened by
" :NERDTreeTabsToggle), default 1
let g:nerdtree_tabs_open_on_new_tab=1
" unfocus NERDTree when leaving a tab for descriptive tab names, default 1.
let g:nerdtree_tabs_meaningful_tab_names=1
" Close current tab if there is only one window in it and it's NERDTree, default 1.
let g:nerdtree_tabs_autoclose=1
" Synchronize view of all NERDTree windows (scroll and cursor position), default 1.
let g:nerdtree_tabs_synchronize_view=1
" Synchronize focus when switching windows (focus NERDTree after tab switch
" if and only if it was focused before tab switch), default 1.
let g:nerdtree_tabs_synchronize_focus=1
" When switching into a tab, make sure that focus is on the file window,
" not in the NERDTree window. (Note that this can get annoying if you use
" NERDTree's feature "open in new tab silently", as you will lose focus on the
" NERDTree.), default 0.
let g:nerdtree_tabs_focus_on_files=0
" When given a directory name as a command line parameter when launching Vim,
" `:cd` into it, default 1.
let g:nerdtree_tabs_startup_cd=1
" Automatically find and select currently opened file in NERDTree, default 0.
let g:nerdtree_tabs_autofind=0
" }1

" {1   plugin: vim-powerline/airline
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"let g:Powerline_colorscheme='solarized256'
"set guifont=DejaVu\ Sans\ Mono\ for\ Powerline\ 9
let g:airline_powerline_fonts = 0
" enable tabline on the top
let g:airline#extensions#tabline#enabled = 1
" set left separator
let g:airline#extensions#tabline#left_sep = '>'
" set left separator which are not editting
let g:airline#extensions#tabline#left_alt_sep = '|'
" show buffer number
let g:airline#extensions#tabline#buffer_nr_show = 1
" Smarter tab line
let g:airline#extensions#tabline#enabled = 1
let g:airline#extensions#tabline#left_sep = ' '
let g:airline#extensions#tabline#left_alt_sep = '|'
" raven, sol, serene
let g:airline_theme='serene'
"let g:airline_section_b = '%{strftime("%c")}'
"let g:airline_section_y = 'BN: %{bufnr("%")}'
"let g:airline#extensions#tavline#enabled = 1
"let g:Powerline_symbols='fancy'
" }1

" {1   plugin: xterm-color-table
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
":XtermColorTable
" }1

" {1   plugin: numbers.vim
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" plugins that do NOT want to display numbers.
" let g:numbers_exclude = ['unite', 'tagbar', 'startify', 'gundo', 'vimshell', 'w3m']
let g:numbers_exclude = ['tagbar', 'gundo', 'minibufexpl', 'nerdtree']
"nnoremap <F3> :NumbersToggle<CR>
"nnoremap <F4> :NumbersOnOff<CR>
" }1

" {1   plugin: vim-startify
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" }1

" {1   plugin: fencview  FencAutoDetect
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" using F
let g:fencview_autodetect=0      " auto-detect when load
let g:fencview_checklines=10     " only test the first 10 lines, or '*' for all.
" }1

" {1   plugin: UltiSnips
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
" Trigger configuration. Do not use <tab> if you use YouCompleteMe.
let g:UltiSnipsExpandTrigger="<tab>"
let g:UltiSnipsJumpForwardTrigger="<c-f>"
let g:UltiSnipsJumpBackwardTrigger="<c-b>"
" If you want :UltiSnipsEdit to split your window.
let g:UltiSnipsEditSplit="vertical"
" }1

" {1   plugin: YouCompleteMe
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"let g:loaded_youcompleteme = 1
""let g:deoplete#enable_at_startup=1
"set completeopt=longest,menu  " let the actions of auto-complete list be same with other IDE.
                              " details VimTip1228.
let g:acp_enableAtStartup = 0
let g:ycm_server_python_interpreter='/usr/bin/python'
let g:ycm_global_ycm_extra_conf='~/.vim/.ycm_extra_conf.py'

let g:ycm_server_keep_logfiles = 0
let g:ycm_server_log_level = 'info'

let g:ycm_cache_omnifunc = 0
let g:ycm_seed_identifiers_with_syntax = 1
let g:ycm_min_num_of_chars_for_completion= 2
let g:ycm_collect_identifiers_from_tags_files = 1             " 使用ctags生成的tags文件
let g:ycm_collect_identifiers_from_comments_and_strings = 1
let g:ycm_use_ultisnips_completer=1
let g:ycm_key_list_select_completion=[]
let g:ycm_key_list_previous_completion=[]
let g:ycm_confirm_extra_conf=0
set completeopt-=preview

let g:ycm_error_symbol = '>>'
let g:ycm_warning_symbol = '>*'
let g:ycm_complete_in_comments = 1   " 注释输入中也能补全
let g:ycm_complete_in_strings = 1    " 字符串输入中也能补全
let g:ycm_collect_identifiers_from_comments_and_strings = 0   " 注释和字符串中的文字不会被收入补全

nnoremap <leader>gl :YcmCompleter GoToDeclaration<CR>
nnoremap <leader>gf :YcmCompleter GoToDefinition<CR>
nnoremap <leader>gg :YcmCompleter GoToDefinitionElseDeclaration<CR>
nnoremap <F5> :YcmForceCompileAndDiagnostics<CR>" force recomile with syntastic
nmap <F6> :YcmDiags<CR>
" }1

" {1   plugin: Syntastic
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let g:syntastic_ignore_files=[".*\.py$"]    " ignore python files
" }1

" {1   plugin: tags...
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set tags+=./tags,./../tags,./../../tags,./../../../tags,./../../../../tags,./../../../../../tags
" generate and include system tags.
"ctags -I __THROW -I __attribute_pure__ -I __nonnull -I __attribute__ \
"  --file-scope=yes              \
"  --langmap=c:+.h               \
"  --languages=c,c++             \
"  --links=yes                   \
"  --c-kinds=+p --c++-kinds=+p   \
"  --fields=+iaS --extra=+q -R -f ~/.vim/systags /usr/include /usr/local/include
set tags+=~/.vim/systags
" use Ctrl-F12 to generate tags.
map <C-F12> :!ctags -R --c-kinds=+px --fields=+iaS --extra=+q <CR>
function! AutoLoadCTagsAndCScope()
    let max = 10
    let dir = './'
    let i = 0
    let break = 0
    while isdirectory(dir) && i < max
        if filereadable(dir . 'GTAGS')
            execute 'cs add ' . dir . 'GTAGS ' . glob("`pwd`")
            let break = 1
        endif
        if filereadable(dir . 'cscope.out')
            execute 'cs add ' . dir . 'cscope.out'
            let break = 1
        endif
        if filereadable(dir . 'tags')
            execute 'set tags =' . dir . 'tags'
            let break = 1
        endif
        if break == 1
            execute 'lcd ' . dir
            break
        endif
        let dir = dir . '../'
        let i = i + 1
    endwhile
endf
nmap <F7> :call AutoLoadCTagsAndCScope()<CR>
" call AutoLoadCTagsAndCScope()
" http://vifix.cn/blog/vim-auto-load-ctags-and-cscope.html
" }1












" {1   plugin: vim-togglelist  quickfix:<leader>q, location:<leader>l
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
let g:toggle_list_no_mappings=1
nmap <script> <silent> <leader>l :call ToggleLocationList()<CR>
nmap <script> <silent> <leader>q :call ToggleQuickfixList()<CR>
" specify the command to open quickfix, always open at bottom(default on the right bottom),
" 'botright copen 10' specify the height.
let g:toggle_list_copen_command="botright copen"
" After opening or closing either list, the previous window is restored so you can still use `<C-w>p`.
" }1




""" {1   plugin: NeoComplCache
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
""" do NOT load plugin, unless commented.
"""let g:loaded_neocomplcache=1
""
"""Note: This option must set it in .vimrc(_vimrc).  NOT IN .gvimrc(_gvimrc)!
""" Disable AutoComplPop.
""let g:acp_enableAtStartup = 0
""" Use neocomplcache.
""let g:neocomplcache_enable_at_startup = 1
""" Use smartcase.
""let g:neocomplcache_enable_smart_case = 1
""" Set minimum syntax keyword length.
""let g:neocomplcache_min_syntax_length = 3
""let g:neocomplcache_lock_buffer_name_pattern = '\*ku\*'
""
""" Enable heavy features.
""" Use camel case completion.
"""let g:neocomplcache_enable_camel_case_completion = 1
""" Use underbar completion.
"""let g:neocomplcache_enable_underbar_completion = 1
""
""" Define dictionary.
""let g:neocomplcache_dictionary_filetype_lists = {
""    \ 'default' : '',
""    \ 'vimshell' : $HOME.'/.vimshell_hist',
""    \ 'scheme' : $HOME.'/.gosh_completions'
""        \ }
""
""" Define keyword.
""if !exists('g:neocomplcache_keyword_patterns')
""    let g:neocomplcache_keyword_patterns = {}
""endif
""let g:neocomplcache_keyword_patterns['default'] = '\h\w*'
""
""" Plugin key-mappings.
""inoremap <expr><C-g>     neocomplcache#undo_completion()
""inoremap <expr><C-l>     neocomplcache#complete_common_string()
""
""" Recommended key-mappings.
""" <CR>: close popup and save indent.
""inoremap <silent> <CR> <C-r>=<SID>my_cr_function()<CR>
""function! s:my_cr_function()
""  return neocomplcache#smart_close_popup() . "\<CR>"
""  " For no inserting <CR> key.
""  "return pumvisible() ? neocomplcache#close_popup() : "\<CR>"
""endfunction
""" <TAB>: completion.
""inoremap <expr><TAB>  pumvisible() ? "\<C-n>" : "\<TAB>"
""" <C-h>, <BS>: close popup and delete backword char.
""inoremap <expr><C-h> neocomplcache#smart_close_popup()."\<C-h>"
""inoremap <expr><BS> neocomplcache#smart_close_popup()."\<C-h>"
""inoremap <expr><C-y>  neocomplcache#close_popup()
""inoremap <expr><C-e>  neocomplcache#cancel_popup()
""" Close popup by <Space>.
"""inoremap <expr><Space> pumvisible() ? neocomplcache#close_popup() : "\<Space>"
""
""" For cursor moving in insert mode(Not recommended)
"""inoremap <expr><Left>  neocomplcache#close_popup() . "\<Left>"
"""inoremap <expr><Right> neocomplcache#close_popup() . "\<Right>"
"""inoremap <expr><Up>    neocomplcache#close_popup() . "\<Up>"
"""inoremap <expr><Down>  neocomplcache#close_popup() . "\<Down>"
""" Or set this.
"""let g:neocomplcache_enable_cursor_hold_i = 1
""" Or set this.
"""let g:neocomplcache_enable_insert_char_pre = 1
""
""" AutoComplPop like behavior.
"""let g:neocomplcache_enable_auto_select = 1
""
""" Shell like behavior(not recommended).
"""set completeopt+=longest
"""let g:neocomplcache_enable_auto_select = 1
"""let g:neocomplcache_disable_auto_complete = 1
"""inoremap <expr><TAB>  pumvisible() ? "\<Down>" : "\<C-x>\<C-u>"
""
""" Enable omni completion.
""autocmd FileType css setlocal omnifunc=csscomplete#CompleteCSS
""autocmd FileType html,markdown setlocal omnifunc=htmlcomplete#CompleteTags
""autocmd FileType javascript setlocal omnifunc=javascriptcomplete#CompleteJS
""autocmd FileType python setlocal omnifunc=pythoncomplete#Complete
""autocmd FileType xml setlocal omnifunc=xmlcomplete#CompleteTags
""
""" Enable heavy omni completion.
""if !exists('g:neocomplcache_force_omni_patterns')
""  let g:neocomplcache_force_omni_patterns = {}
""endif
""let g:neocomplcache_force_omni_patterns.php = '[^. \t]->\h\w*\|\h\w*::'
""let g:neocomplcache_force_omni_patterns.c = '[^.[:digit:] *\t]\%(\.\|->\)'
""let g:neocomplcache_force_omni_patterns.cpp = '[^.[:digit:] *\t]\%(\.\|->\)\|\h\w*::'
""
""" For perlomni.vim setting.
""" https://github.com/c9s/perlomni.vim
""let g:neocomplcache_force_omni_patterns.perl = '\h\w*->\h\w*\|\h\w*::'
""" }1
"
"" {1   plugin: EasyGrep
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
""let g:EasyGrepVersion = "1.2"    " do NOT load easygrep
"" }1
"
"" {1   plugin: NeoSnippet
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" do NOT load plugin, unless commented.
"let g:loaded_neosnippet=0
"" }1
"
"" {1   plugin: NerdCommenter NO
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" }1
"
"" {1   plugin: SrcExpl
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" do NOT load plugin, unless commented.
"let g:loaded_neocomplcach=0
"" }1
"
"" {1   plugin: EasyMotion
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" do NOT load plugin, unless commented.
"let g:EasyMotion_loaded=0
"" }1
"
"" {1   plugin: python-mode
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" K             Show python docs
"" <Ctrl-Space>  Rope autocomplete
"" <Ctrl-c>g     Rope goto definition
"" <Ctrl-c>d     Rope show documentation
"" <Ctrl-c>f     Rope find occurrences
"" <Leader>b     Set, unset breakpoint (g:pymode_breakpoint enabled)
"" [[            Jump on previous class or function (normal, visual, operator modes)
"" ]]            Jump on next class or function (normal, visual, operator modes)
"" [M            Jump on previous class or method (normal, visual, operator modes)
"" ]M            Jump on next class or method (normal, visual, operator modes)
"
"" load python-mode plugin 1, or not 0.
"let g:pymode = 0
"
"
"let g:pymode_rope = 1
"
"
"" Documentation
"let g:pymode_doc = 1
"let g:pymode_doc_key = 'K'
"
""Linting
"let g:pymode_lint = 1
"let g:pymode_lint_checker = "pyflakes, pep8"
"let g:flake8_ignore="E401"
"" Auto check on save
"let g:pymode_lint_write = 1
"
"" Support virtualenv
"let g:pymode_virtualenv = 1
"
"" Enable breakpoints plugin
"let g:pymode_breakpoint = 1
"let g:pymode_breakpoint_key = '<leader>b'
"
"" syntax highlighting
"let g:pymode_syntax = 1
"let g:pymode_syntax_all = 1
"let g:pymode_syntax_indent_errors = g:pymode_syntax_all
"let g:pymode_syntax_space_errors = g:pymode_syntax_all
"
"" Don't autofold code
"let g:pymode_folding = 0
"" }1
"
"" {1   plugin: cscope
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
""
"" This file contains some boilerplate settings for vim's cscope interface,
"" plus some keyboard mappings that I've found useful.
""
"" USAGE:
"" -- vim 6:     Stick this file in your ~/.vim/plugin directory (or in a
""               'plugin' directory in some other directory that is in your
""               'runtimepath'.
""
"" -- vim 5:     Stick this file somewhere and 'source cscope.vim' it from
""               your ~/.vimrc file (or cut and paste it into your .vimrc).
""
"" NOTE:
"" These key maps use multiple keystrokes (2 or 3 keys).  If you find that vim
"" keeps timing you out before you can complete them, try changing your timeout
"" settings, as explained below.
""
"" Happy cscoping,
""
"" Jason Duell       jduell@alumni.princeton.edu     2002/3/7
"" A copy from http://cscope.sourceforge.net/cscope_maps.vim
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"
"
"" This tests to see if vim was configured with the '--enable-cscope' option
"" when it was compiled.  If it wasn't, time to recompile vim...
"if has("cscope")
"
"    """"""""""""" Standard cscope/vim boilerplate
"    " set the program of cscope.
"    set cscopeprg=cscope
"
"    " use both cscope and ctag for 'ctrl-]', ':ta', and 'vim -t'
"    set cscopetag
"
"    " check cscope for definition of a symbol before checking ctags: set to 1
"    " if you want the reverse search order.
"    set csto=0
""    " some where else added.
""    " add any cscope database in current directory
""    if filereadable("cscope.out")
""        cs add cscope.out
""    " else add the database pointed to by environment variable
""    elseif $CSCOPE_DB != ""
""        cs add $CSCOPE_DB
""    endif
"
"    " when type :cstag, tags first then cscope.
"    set cscopetagorder=1
"
"    " show msg when any other cscope db added
"    set cscopeverbose
"
"    set cscopequickfix=s-,c-,d-,i-,t-,e-
"
"    """"""""""""" My cscope/vim key mappings
"    "
"    " The following maps all invoke one of the following cscope search types:
"    "
"    "   's'   symbol: find all references to the token under cursor
"    "   'g'   global: find global definition(s) of the token under cursor
"    "   'c'   calls:  find all calls to the function name under cursor
"    "   't'   text:   find all instances of the text under cursor
"    "   'e'   egrep:  egrep search for the word under cursor
"    "   'f'   file:   open the filename under cursor
"    "   'i'   includes: find files that include the filename under cursor
"    "   'd'   called: find functions that function under cursor calls
"    "
"    " Below are three sets of the maps: one set that just jumps to your
"    " search result, one that splits the existing vim window horizontally and
"    " diplays your search result in the new window, and one that does the same
"    " thing, but does a vertical split instead (vim 6 only).
"    "
"    " I've used CTRL-\ and CTRL-@ as the starting keys for these maps, as it's
"    " unlikely that you need their default mappings (CTRL-\'s default use is
"    " as part of CTRL-\ CTRL-N typemap, which basically just does the same
"    " thing as hitting 'escape': CTRL-@ doesn't seem to have any default use).
"    " If you don't like using 'CTRL-@' or CTRL-\, , you can change some or all
"    " of these maps to use other keys.  One likely candidate is 'CTRL-_'
"    " (which also maps to CTRL-/, which is easier to type).  By default it is
"    " used to switch between Hebrew and English keyboard mode.
"    "
"    " All of the maps involving the <cfile> macro use '^<cfile>$': this is so
"    " that searches over '#include <time.h>" return only references to
"    " 'time.h', and not 'sys/time.h', etc. (by default cscope will return all
"    " files that contain 'time.h' as part of their name).
"
"
"    " To do the first type of search, hit 'CTRL-\', followed by one of the
"    " cscope search types above (s,g,c,t,e,f,i,d).  The result of your cscope
"    " search will be displayed in the current window.  You can use CTRL-T to
"    " go back to where you were before the search.
"    "
"
"    nmap <C-\>s :cs find s <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-\>g :cs find g <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-\>c :cs find c <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-\>t :cs find t <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-\>e :cs find e <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-\>f :cs find f <C-R>=expand("<cfile>")<CR><CR>
"    nmap <C-\>i :cs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
"    nmap <C-\>d :cs find d <C-R>=expand("<cword>")<CR><CR>
"
"
"    " Using 'CTRL-spacebar' (intepreted as CTRL-@ by vim) then a search type
"    " makes the vim window split horizontally, with search result displayed in
"    " the new window.
"    "
"    " (Note: earlier versions of vim may not have the :scs command, but it
"    " can be simulated roughly via:
"    "    nmap <C-@>s <C-W><C-S> :cs find s <C-R>=expand("<cword>")<CR><CR>
"
"    nmap <C-@>s :scs find s <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@>g :scs find g <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@>c :scs find c <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@>t :scs find t <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@>e :scs find e <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@>f :scs find f <C-R>=expand("<cfile>")<CR><CR>
"    nmap <C-@>i :scs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
"    nmap <C-@>d :scs find d <C-R>=expand("<cword>")<CR><CR>
"
"
"    " Hitting CTRL-space *twice* before the search type does a vertical
"    " split instead of a horizontal one (vim 6 and up only)
"    "
"    " (Note: you may wish to put a 'set splitright' in your .vimrc
"    " if you prefer the new window on the right instead of the left
"
"    nmap <C-@><C-@>s :vert scs find s <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@><C-@>g :vert scs find g <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@><C-@>c :vert scs find c <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@><C-@>t :vert scs find t <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@><C-@>e :vert scs find e <C-R>=expand("<cword>")<CR><CR>
"    nmap <C-@><C-@>f :vert scs find f <C-R>=expand("<cfile>")<CR><CR>
"    nmap <C-@><C-@>i :vert scs find i ^<C-R>=expand("<cfile>")<CR>$<CR>
"    nmap <C-@><C-@>d :vert scs find d <C-R>=expand("<cword>")<CR><CR>
"
"
"    """"""""""""" key map timeouts
"    "
"    " By default Vim will only wait 1 second for each keystroke in a mapping.
"    " You may find that too short with the above typemaps.  If so, you should
"    " either turn off mapping timeouts via 'notimeout'.
"    "
"    "set notimeout
"    "
"    " Or, you can keep timeouts, by uncommenting the timeoutlen line below,
"    " with your own personal favorite value (in milliseconds):
"    "
"    "set timeoutlen=4000
"    "
"    " Either way, since mapping timeout settings by default also set the
"    " timeouts for multicharacter 'keys codes' (like <F1>), you should also
"    " set ttimeout and ttimeoutlen: otherwise, you will experience strange
"    " delays as vim waits for a keystroke after you hit ESC (it will be
"    " waiting to see if the ESC is actually part of a key code like <F1>).
"    "
"    "set ttimeout
"    "
"    " personally, I find a tenth of a second to work well for key code
"    " timeouts. If you experience problems and have a slow terminal or network
"    " connection, set it higher.  If you don't set ttimeoutlen, the value for
"    " timeoutlent (default: 1000 = 1 second, which is sluggish) is used.
"    "
"    "set ttimeoutlen=100
"
"endif
"" }1
"
"" {1   plugin: SrcExpl
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" // The switch of the Source Explorer
"nmap <F8> :SrcExplToggle<CR>
"
"" // Set the height of Source Explorer window
"let g:SrcExpl_winHeight = 8
"
"" // Set 100 ms for refreshing the Source Explorer
"let g:SrcExpl_refreshTime = 100
"
"" // Set "Enter" key to jump into the exact definition context
"let g:SrcExpl_jumpKey = "<ENTER>"
"
"" // Set "Space" key for back from the definition context
"let g:SrcExpl_gobackKey = "<SPACE>"
"
"" // In order to avoid conflicts, the Source Explorer should know what plugins
"" // except itself are using buffers. And you need add their buffer names into
"" // below listaccording to the command ":buffers!"
"let g:SrcExpl_pluginList = [
"        \ "__Tagbar__",
"        \ "_NERD_tree_",
"        \ "Source_Explorer"
"    \ ]
"
"" // Enable/Disable the local definition searching, and note that this is not
"" // guaranteed to work, the Source Explorer doesn't check the syntax for now.
"" // It only searches for a match with the keyword according to command 'gd'
"let g:SrcExpl_searchLocalDef = 1
"
"" // Do not let the Source Explorer update the tags file when opening
"let g:SrcExpl_isUpdateTags = 0
"
"" // Use 'Exuberant Ctags' with '--sort=foldcase -R .' or '-L cscope.files' to
"" // create/update the tags file
""let g:SrcExpl_updateTagsCmd = "ctags --sort=foldcase -R ."
"let g:SrcExpl_updateTagsCmd = ""
"
"" // Set "<F12>" key for updating the tags file artificially
"let g:SrcExpl_updateTagsKey = "<F12>"
"
"" // Set "<F3>" key for displaying the previous definition in the jump list
"let g:SrcExpl_prevDefKey = "<F3>"
"
"" // Set "<F4>" key for displaying the next definition in the jump list
"let g:SrcExpl_nextDefKey = "<F4>"
"" }1
"
""" {1   plugin: WinManager
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"""let loaded_winmanager = 0           " winmanager not loaded
""let g:winManagerWidth = 30          " the width of WinManager, default 30
""nmap <silent> <F8> :WMToggle<cr>    " open/close winmanager through <F8>
""nmap wm :WMToggle<cr>               " equal to 'wm'
""let g:persistentBehaviour=0         " quit when only one window
""let g:winManagerWindowLayout = "NERDTree|FileExplorer,BufExplorer"
""let g:bufExplorerMaxHeight=30
""let g:AutoOpenWinManager=0        " donot open WinManager when vim start.
""""let g:defaultExplorer=1
"""map <c-w><c-f> :FirstExplorerWindow<cr>
"""map <c-w><c-b> :BottomExplorerWindow<cr>
""" }1

" Visual mode pressing * or # searches for the current selection
" Super useful! From an idea by Michael Naumann
vnoremap <silent> * :call VisualSelection('f')<CR>
vnoremap <silent> # :call VisualSelection('b')<CR>

"set clipboard=unnamedplus

nnoremap <leader>N :setlocal number!<cr>     " toggle line munber through ,N

au BufRead,BufNewFile *.go set filetype=go

"""""""""""" some setting according to different filetype
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"                                 status line
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set laststatus=2












"" To insert timestamp, press F4.
""nmap <F4> a<C-R>=strftime("%Y-%m-%d %a %I:%M %p")<CR><Esc>
""imap <F4> <C-R>=strftime("%Y-%m-%d %a %I:%M %p")<CR>
"" delete the white space in the end of line
nmap <F2> :%s/\s\+$//g<CR>
imap <F2> :%s/\s\+$//g<CR>
" to save, ctrl-s
nmap <C-S> :w<CR>
imap <C-S> <Esc>:w<CR>a
vmap <C-S> <Esc>:w<CR>
" when try to paste codes, errors will occur, 'set paste' before paste
" or 'set nopaste'.
set nopaste
set pastetoggle=<F9>
"
"augroup vimrc_autocmds
"    autocmd!
"    " highlight characters past column 120
"    autocmd FileType python highlight Excess ctermbg=DarkGrey guibg=Black
"    autocmd FileType python match Excess /\%120v.*/
"    autocmd FileType python set nowrap
"    augroup END
"
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" => Editing mappings
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" Remap VIM 0 to first non-blank character
"map 0 ^
"
"" Move a line of text using ALT+[jk] or Comamnd+[jk] on mac
"nmap <M-j> mz:m+<cr>`z
"nmap <M-k> mz:m-2<cr>`z
"vmap <M-j> :m'>+<cr>`<my`>mzgv`yo`z
"vmap <M-k> :m'<-2<cr>`>my`<mzgv`yo`z
"
"if has("mac") || has("macunix")
"  nmap <D-j> <M-j>
"  nmap <D-k> <M-k>
"  vmap <D-j> <M-j>
"  vmap <D-k> <M-k>
"endif
"
"" Delete trailing white space on save, useful for Python and CoffeeScript ;)
"func! DeleteTrailingWS()
"  exe "normal mz"
"  %s/\s\+$//ge
"  exe "normal `z"
"endfunc
"autocmd BufWrite *.py :call DeleteTrailingWS()
"autocmd BufWrite *.coffee :call DeleteTrailingWS()
"
"
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" => vimgrep searching and cope displaying
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" When you press gv you vimgrep after the selected text
"vnoremap <silent> gv :call VisualSelection('gv')<CR>
"
"" Open vimgrep and put the cursor in the right position
"map <leader>g :vimgrep // **/*.<left><left><left><left><left><left><left>
"
"" Vimgreps in the current file
"map <leader><space> :vimgrep // <C-R>%<C-A><right><right><right><right><right><right><right><right><right>
"
"" When you press <leader>r you can search and replace the selected text
"vnoremap <silent> <leader>r :call VisualSelection('replace')<CR>
"
"" Do :help cope if you are unsure what cope is. It's super useful!
""
"" When you search with vimgrep, display your results in cope by doing:
""   <leader>cc
""
"" To go to the next search result do:
""   <leader>n
""
"" To go to the previous search results do:
""   <leader>p
""
"map <leader>cc :botright cope<cr>
"map <leader>co ggVGy:tabnew<cr>:set syntax=qf<cr>pgg
"map <leader>n :cn<cr>
"map <leader>p :cp<cr>
"
"
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" => Spell checking
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" Pressing ,ss will toggle and untoggle spell checking
"map <leader>ss :setlocal spell!<cr>
"
"" Shortcuts using <leader>
"map <leader>sn ]s
"map <leader>sp [s
"map <leader>sa zg
"map <leader>s? z=
"
"
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" => Misc
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" Remove the Windows ^M - when the encodings gets messed up
"noremap <Leader>m mmHmt:%s/<C-V><cr>//ge<cr>'tzt'm
"
"" Quickly open a buffer for scripbble
"map <leader>q :e ~/buffer<cr>
"
"" Toggle paste mode on and off
"map <leader>pp :setlocal paste!<cr>
"
"
"
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"" => Helper functions
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"function! CmdLine(str)
"    exe "menu Foo.Bar :" . a:str
"    emenu Foo.Bar
"    unmenu Foo
"endfunction
"
"function! VisualSelection(direction) range
"    let l:saved_reg = @"
"    execute "normal! vgvy"
"
"    let l:pattern = escape(@", '\\/.*$^~[]')
"    let l:pattern = substitute(l:pattern, "\n$", "", "")
"
"    if a:direction == 'b'
"        execute "normal ?" . l:pattern . "^M"
"    elseif a:direction == 'gv'
"        call CmdLine("vimgrep " . '/'. l:pattern . '/' . ' **/*.')
"    elseif a:direction == 'replace'
"        call CmdLine("%s" . '/'. l:pattern . '/')
"    elseif a:direction == 'f'
"        execute "normal /" . l:pattern . "^M"
"    endif
"
"    let @/ = l:pattern
"    let @" = l:saved_reg
"endfunction
"
"
"" Returns true if paste mode is enabled
"function! HasPaste()
"    if &paste
"        return 'PASTE MODE  '
"    en
"    return ''
"endfunction
"
"" Don't close window, when deleting a buffer
"command! Bclose call <SID>BufcloseCloseIt()
"function! <SID>BufcloseCloseIt()
"   let l:currentBufNum = bufnr("%")
"   let l:alternateBufNum = bufnr("#")
"
"   if buflisted(l:alternateBufNum)
"     buffer #
"   else
"     bnext
"   endif
"
"   if bufnr("%") == l:currentBufNum
"     new
"   endif
"
"   if buflisted(l:currentBufNum)
"     execute("bdelete! ".l:currentBufNum)
"   endif
"endfunction
"


endif
