package pdg;

import ast.declaration.TypedefDeclaration;

/**
 * Created by ooee on 3/9/17.
 */
public class PDGNodeTypedef extends PDGNode<TypedefDeclaration> {
    public PDGNodeTypedef(TypedefDeclaration blockItem) {
        super(blockItem, true);
    }

    @Override
    public TypedefDeclaration sort(PDGSorter sorter) {
        return blockItem;
    }
}
