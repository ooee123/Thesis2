package pdg;

import ast.Declaration;
import ast.ExpressionStatement;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeDeclaration extends PDGNode<Declaration> {

    public PDGNodeDeclaration(Declaration declaration) {
        this(declaration, false);
    }

    public PDGNodeDeclaration(Declaration declaration, boolean required) {
        super(declaration, required);
    }

    @Override
    public Declaration sort(PDGSorter sorter) {
        return blockItem;
    }
}
