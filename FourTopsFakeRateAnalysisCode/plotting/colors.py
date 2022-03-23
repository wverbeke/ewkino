######################
# tools for plotting #
######################

import ROOT


def getcolormap( style='default' ):
    style=style.lower()
    if(style=='default'): return getcolormap_default()
    elif(style=='ukraine'): return getcolormap_ukraine()
    else: print('WARNING in getcolormap: style not recognized, returning None')
    return None

def defineColorHex(hexstring):
    # note: the color object needs to stay in scope, else the index will refer to a nullpointer
    # and the color will turn out white...
    # however, this is solved by adding the color as a global ROOT feature
    # (not very clean, better to work with a custom color class, but good enough for now)
    r, g, b = tuple(int(hexstring[i:i+2], 16) for i in (1, 3, 5))
    cindex = ROOT.TColor.GetFreeColorIndex()
    color = ROOT.TColor(cindex, r/255., g/255., b/255.)
    setattr(ROOT,'temp_color_'+str(cindex),color)
    return cindex,color

def getcolormap_default():
    # map of histogram titles to colors
    cmap = {}
    cmap['nonprompt'] = defineColorHex('#ffe380')[0]
    cmap['DY'] = defineColorHex('#ffd22e')[0]
    cmap['TT'] = defineColorHex('#ffbd80')[0]
    cmap['TTX'] = cmap['TT']
    cmap['TTZ'] = defineColorHex('#336fce')[0]
    cmap['TTW'] = defineColorHex('#2f8ceb')[0]
    cmap['tZq'] = defineColorHex('#ff0000')[0]
    cmap['WZ'] = defineColorHex('#81efd7')[0]
    cmap['ZZ'] = defineColorHex('#2fbc6c')[0]
    cmap['ZZH'] = cmap['ZZ']
    cmap['ZG'] = defineColorHex('#9c88ff')[0]
    cmap['XG'] = cmap['ZG']
    cmap['triboson'] = defineColorHex('#c6ff00')[0]
    cmap['Triboson'] = cmap['triboson']
    cmap['Other'] = defineColorHex('#ccccaa')[0]
    return cmap

def getcolormap_ukraine():
    # yellow and blue
    cmap = {}
    cmap['nonprompt'] = defineColorHex('#ffd700')[0]
    cmap['DY'] = defineColorHex('#0057b7')[0]
    cmap['TT'] = cmap['DY']
    cmap['TTX'] = cmap['DY']
    cmap['TTZ'] = cmap['DY']
    cmap['TTW'] = cmap['DY']
    cmap['tZq'] = cmap['DY']
    cmap['WZ'] = cmap['DY']
    cmap['ZZ'] = cmap['DY']
    cmap['ZZH'] = cmap['DY']
    cmap['ZG'] = cmap['DY']
    cmap['XG'] = cmap['DY']
    cmap['triboson'] = cmap['DY']
    cmap['Triboson'] = cmap['DY']
    cmap['Other'] = cmap['DY']
    return cmap
