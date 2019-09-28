(function($) {
    $.fn.simpleJekyllSearch = function(options) {
        var settings = $.extend({
            jsonFile            : '/search.json',
            template            : '<a href="{url}" title="{desc}">{title}</a>',
            searchResults       : '.results',
            searchResultsTitle  : '<h4>Search results</h4>',
            limit               : '10',
            noResults           : '<p>Oh shucks<br/><small>Nothing found :(</small></p>'
        }, options);
        var jsonData = [],
            origThis = this,
            searchResults = $(settings.searchResults);
        var matches = [];
        if(settings.jsonFile.length && searchResults.length){
            $.ajax({
                type: "GET",
                url: settings.jsonFile,
                dataType: 'json',
                success: function(data, textStatus, jqXHR) {
                    jsonData = data;
                    registerEvent();
                },
                error: function(x,y,z) {
                    console.log("***ERROR in simpleJekyllSearch.js***");
                    console.log(x); // should have what's wrong
                    console.log(y);
                    console.log(z);
                }
            });
        }

        function registerEvent(){
            origThis.keyup(function(e){
                if(e.which === 13){
                    if(matches)
                        window.location = matches[0].url;
                }
                if($(this).val().length){
                    writeMatches( performSearch($(this).val()) );
                }else{
                    clearSearchResults();
                }
            });
        }

        function performSearch(str){
            matches = [];
            for (var i = 0; i < jsonData.length; i++) {
                var obj = jsonData[i];
                for (key in obj) {
                    console.log(key);
                    if (obj.hasOwnProperty(key) && obj[key].toLowerCase().indexOf(str.toLowerCase()) >= 0){
                        matches.push(obj);
                        break;
                    }
                }
            }
            return matches;
        }

        function writeMatches(m){
            clearSearchResults();
            searchResults.append( $(settings.searchResultsTitle) );

            if(m && m.length){
                for (var i = 0; i < m.length && i < settings.limit; i++) {
                    var obj = m[i];
                    output = settings.template;
                    output = output.replace(/\{(.*?)\}/g, function(match, property) {
                        return obj[property];
                    });
                    searchResults.append($(output));
                }
            }else{
                searchResults.append( settings.noResults );
            }
        }
        function clearSearchResults(){
            searchResults.children().remove();
        }
    }
}(jQuery));


jQuery(document).ready(function(){
    // For simple search function.
    $('.search-field').simpleJekyllSearch({
        jsonFile            : '/search.json',
        template            : '<li style="list-style-type:none;"><a href="{url}" title="<category>: {category}; <URL>: {url}">{title}</a></li>',
        searchResults       : '.search-results',
        searchResultsTitle  : '<h4>Search results:</h4>',
        limit               : '15',
        noResults           : '<p>Oh sucks, Nothing found :(</p>'
    });

























    var isMobile = {
        Android: function() {
            return navigator.userAgent.match(/Android/i);
        }
        ,BlackBerry: function() {
            return navigator.userAgent.match(/BlackBerry/i);
        }
        ,iOS: function() {
            return navigator.userAgent.match(/iPhone|iPad|iPod/i);
        }
        ,Opera: function() {
            return navigator.userAgent.match(/Opera Mini/i);
        }
        ,Windows: function() {
            return navigator.userAgent.match(/IEMobile/i);
        }
        ,any: function() {
            return (isMobile.Android() || isMobile.BlackBerry() || isMobile.iOS() || isMobile.Opera() || isMobile.Windows());
        }
    };


    (function(){
        var waitForFinalEvent = (function () {
            var timers = {};
            return function (callback, ms, uniqueId) {
                if (!uniqueId) {
                    uniqueId = "Don't call this twice without a uniqueId";
                }
                if (timers[uniqueId]) {
                    clearTimeout (timers[uniqueId]);
                }
                timers[uniqueId] = setTimeout(callback, ms);
            };
        })();


        if($('.blog-main h2').length > 2 && !isMobile.any()){

            var tmpl = '<p align="center"><a href="#">' + $('.blog-main h1').text(); + '</a></p>';
            tmpl += '<ul>';
            $.each($('.blog-main h2, .blog-main h3'), function(index, item){
                //console.log($(item).text());
                //console.log($(item).attr('id'));
                if(item.tagName.toLowerCase() == 'h2'){
                    tmpl += '<li><a href="#" data-id="'+$(item).attr('id')+'">'+$(item).text()+'</a></li>';
                }else{
                    tmpl += '<li  style="font-size:0.9em;padding-left:1.5em;"><a href="#" data-id="'+$(item).attr('id')+'">'+$(item).text()+'</a></li>';
                }
            });

            var indexCon = '<div id="toc-container"><h2>Table of Contents</h2><div class="sidebar-module" id="toc-contents"></div></div>'
            $('.blog-sidebar').append(indexCon);
            $('#toc-contents').append($(tmpl)).delegate('a','click',function(e) {
                e.preventDefault();
                var selector = $(this).attr('data-id') ? '#'+$(this).attr('data-id') : 'h1';
                // 动态移动到anchor处，可以设置偏移位置，以及动画速度
                $('body, html').animate({ scrollTop: $(selector).offset().top - 70 }, 300, 'swing');
            });

            $(window).load(function(){
                var scrollTop = [];
                $.each($('#toc-contents li a'),function(index,item){
                    var selector = $(item).attr('data-id') ? '#'+$(item).attr('data-id') : 'h1'
                    scrollTop.push($(selector).offset().top);
                });

                var menuindexTop = $('#toc-container').offset().top;
                var menuindexLeft = $('.sidebar-module').offset().left;
                var menuindexwidth = $('.blog-sidebar').width();

                $(window).resize(function(){
                    menuindexLeft = $('.sidebar-module').offset().left;
                    menuindexwidth = $('.blog-sidebar').width();
                    $('#toc-container').css({
                        left:menuindexLeft,
                        width:menuindexwidth
                    });
                    console.log(document.body.clientWidth);
                });

                $(window).scroll(function(){
                    waitForFinalEvent(function(){
                        var nowTop = $(window).scrollTop();
                        var length = scrollTop.length;
                        var index;

                        if(nowTop+20 > menuindexTop){
                            $('#toc-container').css({
                                position:'fixed',
                                top:'30px',
                                left:menuindexLeft,
								width:menuindexwidth
                            });
                        } else {
                            $('#toc-container').css({
                                position:'static',
                                top:0,
                                left:0
                            });
                        }

                        if(nowTop+60 > scrollTop[length-1]){
                            index = length;
                        }else{
                            for(var i=0; i<length; i++){
                                if(nowTop+60 <= scrollTop[i]){
                                    index = i;
                                    break;
                                }
                            }
                        }
                        $('#toc-contents li').removeClass('active');
                        $('#toc-contents li').eq(index-1).addClass('active');
                    });
                });
            })
            //用js计算屏幕的高度
            //$('#toc-contents').css('max-height',$(window).height()-80);
        }
    })();
});
