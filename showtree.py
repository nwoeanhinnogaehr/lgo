from ete3 import *
import sys
t = Tree(sys.argv[1], format=1)
ts = TreeStyle()
ts.show_leaf_name = False
def my_layout(node):
    if node.exact == "0":
        F = TextFace(node.name, ftype="Courier", fgcolor="red")
    else:
        F = TextFace(node.name, ftype="Courier", fgcolor="green")
    add_face_to_node(F, node, column=0, position="branch-right")
ts.layout_fn = my_layout
ts.scale = 10
ts.rotation = 90
t.show(tree_style=ts)
