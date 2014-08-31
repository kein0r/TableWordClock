letter = [['k', 'è','b', 'à', 'n', 'f', 'ē', 'n'],
          ['y', 'ī', 'è', 'r', 'e', 's', 'h', 'í'],
          ['q', 'ī', 'b', 'a', 'd', 'i', 'ǎ', 'n'],
          ['w', 'ǔ', 'k', 'l', 'j', 'i', 'ǔ', 'i'],
          ['s', 'ā', 'n', 's', 'ì', 'l', 'i', 'ù'],
          ['s', 'h', 'í', 'y', 'ī', 'X', 'è', 'r'],
          ['y', 'ī', 'k', 'è', 's', 'f', 'ē', 'n'],
          ['c', 'h', 'à', 'è', 'r', 's', 'h', 'í']]

boxWidthHeight = 9.5
xOffset = 2.5
yOffset = 2.3

#fontString = "/home/jan/.fonts/c/consolab.ttf"
#fontSize = 5.9

fontString = "/usr/share/fonts/truetype/forgottb.ttf"
fontSize = 3.9
# Striche bei e, n zu schmal
fontString = "/usr/share/fonts/truetype/monofont.ttf"
fontSize = 6.5
# +
fontString = "/usr/share/fonts/truetype/DejaVuSansMono-Bold.ttf"

for row in range(0, 8):
  for column in range(0,8):
    if letter[row][column] != ' ':
      plm=FreeCAD.Placement()
      if letter[row][column] != 'X':
        ss=Draft.makeShapeString(String=letter[row][column],FontFile=fontString,Size=fontSize,Tracking=0)
        plm.Base=FreeCAD.Vector(column * boxWidthHeight + xOffset, row * boxWidthHeight + yOffset, 0.0)
      else:
        ss=Draft.makeShapeString(String=u"\u4e24",FontFile="/usr/share/fonts/truetype/DroidSansFallbackFull.ttf",Size=fontSize / 37.72 * 6,Tracking=0)
        plm.Base=FreeCAD.Vector(column * boxWidthHeight + xOffset-2.4, row * boxWidthHeight + yOffset-0.2,2, 0.0)
      plm.Rotation.Q=(0.0, 0.0, 0.0, 1.0)
      ss.Placement=plm
      ss.Support=None
