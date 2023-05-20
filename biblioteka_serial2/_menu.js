<!--
// ---------------------------------------------------------- 
// MENU MOUSE OVER 
function menuOver() 
{
    clearTimeout(timeOn)
    menuActive = 1
}

// ---------------------------------------------------------- 
// MENU MOUSE OUT 
function menuOut(itemName) 
{
    if(document.layers) 
    {
        menuActive = 0 
        timeOn = setTimeout("hideAllMenus()", 400)
    }
}

// ---------------------------------------------------------- 
// SET BACKGROUND COLOR 
function getImage(name) 
{
    if (document.layers) 
    {
        return findImage(name, document);
    }
    return null;
}

// ---------------------------------------------------------- 
function findImage(name, doc) 
{
    var i, img;
    for (i = 0; i < doc.images.length; i++)
        if (doc.images[i].name == name)
            return doc.images[i];
    
    for (i = 0; i < doc.layers.length; i++)
        if ((img = findImage(name, doc.layers[i].document)) != null) 
        {
            img.container = doc.layers[i];
            return img;
        }
    return null;
}

// ---------------------------------------------------------- 
function getImagePageLeft(img) 
{
    var x, obj;
    if (document.layers) 
    {
        if (img.container != null)
            return img.container.pageX + img.x;
        else
            return img.x;
    }
    
    return -1;
}

// ---------------------------------------------------------- 
function getImagePageTop(img) 
{
    var y, obj;
    if (document.layers) 
    {
        if (img.container != null)
            return img.container.pageY + img.y;
        else
            return img.y;
    }
    
    return -1;
}

//document.write('<style> .menu{position: absolute;}</style>');
var timeOn = null
document.onmouseover = hideAllMenus;
document.onclick = hideAllMenus;
window.onerror = null;

// ---------------------------------------------------------- 
function getStyleObject(objectId)
{
    // cross-browser function to get an object's style object given its id
    if(document.getElementById && document.getElementById(objectId)) 
    {  
        // W3C DOM
        return document.getElementById(objectId).style;
    } 
    else if (document.all && document.all(objectId)) 
    {
        // MSIE 4 DOM
        return document.all(objectId).style;
    } 
    else if (document.layers && document.layers[objectId]) 
    {
        // NN 4 DOM.. note: this won't find nested layers
        return document.layers[objectId];
    } 
    else
    {
        return false;
    }
} // getStyleObject

// ---------------------------------------------------------- 
function changeObjectVisibility(objectId, newVisibility) 
{
    // get a reference to the cross-browser style object and make sure the object exists
    var styleObject = getStyleObject(objectId);
    if(styleObject) 
    {
        styleObject.visibility = newVisibility;
        return true;
    } 
    else 
    {
        //we couldn't find the object, so we can't change its visibility
        return false;
    }
} // changeObjectVisibility


// ---------------------------------------------------------- 
function showMenu(menuNumber, eventObj) 
{
    hideAllMenus();
    if(document.layers) 
    {
        img = getImage("img" + menuNumber);
        x = getImagePageLeft(img);
        y = getImagePageTop(img);
        menuLeft  = x + 0; 
 	    menuTop   = y + 10; 
	    eval('document.layers["menu'+menuNumber+'"].top="'+menuTop+'"');
 	    eval('document.layers["menu'+menuNumber+'"].left="'+menuLeft+'"');
    }
    
    eventObj.cancelBubble = true;
    var menuId = 'menu' + menuNumber;
    
    if(changeObjectVisibility(menuId, 'visible')) 
    {
        return true;
    } 
        else 
    {
        return false;
    }
}
// ---------------------------------------------------------- 
function mene(url) 
{
    location.href=url;	
}
// ---------------------------------------------------------- 
function hideAllMenus() 
{
    for(counter = 1; counter <= numMenus; counter++) 
    {
        changeObjectVisibility('menu' + counter, 'hidden');
    }
}

// ---------------------------------------------------------- 
function moveObject(objectId, newXCoordinate, newYCoordinate) 
{
    // get a reference to the cross-browser style object and make sure the object exists
    var styleObject = getStyleObject(objectId);
    if(styleObject) 
    {
        styleObject.left = newXCoordinate;
        styleObject.top = newYCoordinate;
        return true;
    } 
    else 
    {
        // we couldn't find the object, so we can't very well move it
        return false;
    }
} // moveObject



// ---------------------------------------------------------- 
// ***********************
// hacks and workarounds *
// ***********************

// initialize hacks whenever the page loads
window.onload = initializeHacks;

// ---------------------------------------------------------- 
// setup an event handler to hide popups for generic clicks on the document
function initializeHacks() 
{
    // this ugly little hack resizes a blank div to make sure you can click
    // anywhere in the window for Mac MSIE 5
    if ((navigator.appVersion.indexOf('MSIE 5') != -1) 
        && (navigator.platform.indexOf('Mac') != -1)
        && getStyleObject('blankDiv')) 
    {
        window.onresize = explorerMacResizeFix;
    }
    resizeBlankDiv();
    // this next function creates a placeholder object for older browsers
    createFakeEventObj();
}



// ---------------------------------------------------------- 
function createFakeEventObj() 
{
    // create a fake event object for older browsers to avoid errors in function call
    // when we need to pass the event object to functions
    if (!window.event) 
    {
        window.event = false;
    }
}



// ---------------------------------------------------------- 
function resizeBlankDiv() 
{
    // resize blank placeholder div so IE 5 on mac will get all clicks in window
    if ((navigator.appVersion.indexOf('MSIE 5') != -1) 
        && (navigator.platform.indexOf('Mac') != -1)
        && getStyleObject('blankDiv')) 
    {
        getStyleObject('blankDiv').width = document.body.clientWidth - 20;
        getStyleObject('blankDiv').height = document.body.clientHeight - 20;
    }
}

// ---------------------------------------------------------- 
function explorerMacResizeFix () 
{
    location.reload(false);
}

// ---------------------------------------------------------- 
function mClk(src)
{ 
    if(event.srcElement.tagName=='TD')
        src.children.tags('A')[0].click();
}
//-->
