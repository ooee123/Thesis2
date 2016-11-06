package pdg;

import ast.VariableDeclaration;
import ast.ExpressionStatement;
import ast.VariableDeclaration;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeDeclaration extends PDGNode<VariableDeclaration> {

    public PDGNodeDeclaration(VariableDeclaration variableDeclaration) {
        this(variableDeclaration, false);
    }

    public PDGNodeDeclaration(VariableDeclaration variableDeclaration, boolean required) {
        super(variableDeclaration, required);
    }

    @Override
    public VariableDeclaration sort(PDGSorter sorter) {
        return blockItem;
    }
}
