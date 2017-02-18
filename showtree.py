from ete3 import *
import sys
t = Tree(sys.argv[1], format=1)
ts = TreeStyle()
ts.show_leaf_name = False
def my_layout(node):
        F = TextFace(node.name, ftype="Courier")
        add_face_to_node(F, node, column=0, position="branch-right")
ts.layout_fn = my_layout
t.show(tree_style=ts)
