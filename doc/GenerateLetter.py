
letter = [['k', 'è','b', 'à', 'n', 'f', 'ē', 'n'],
          ['y', 'ī', 'è', 'r', ' ', 's', 'h', 'í'],
          ['q', 'ī', 'b', 'a', 'd', 'i', 'ǎ', 'n'],
          ['w', 'ǔ', ' ', ' ', 'j', 'i', 'ǔ', ' '],
          ['s', 'ā', 'n', 's', 'ì', 'l', 'i', 'ù'],
          ['s', 'h', 'í', 'y', 'ī', 'X', 'è', 'r'],
          ['y', 'ī', 'k', 'è', ' ', 'f', 'ē', 'n'],
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
      ss=Draft.makeShapeString(String=letter[row][column],FontFile=fontString,Size=fontSize,Tracking=0)
      plm=FreeCAD.Placement()
      plm.Base=FreeCAD.Vector(column * boxWidthHeight + xOffset, row * boxWidthHeight + yOffset, 0.0)
      plm.Rotation.Q=(0.0, 0.0, 0.0, 1.0)
      ss.Placement=plm
      ss.Support=None
