######################
# tools for plotting #
######################

import ROOT


def getcolormap( style='default' ):
    style=style.lower()
    if(style=='default'): return getcolormap_default()
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
    cmap['WZ'] = defineColorHex('#81efd7')[0]
    cmap['ZZ'] = defineColorHex('#2fbc6c')[0]
    cmap['ZZH'] = cmap['ZZ']
    cmap['ZG'] = defineColorHex('#9c88ff')[0]
    cmap['XG'] = cmap['ZG']
    cmap['triboson'] = defineColorHex('#c6ff00')[0]
    cmap['Other'] = defineColorHex('#336fce')[0]
    return cmap
