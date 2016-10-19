package pdg;

import ast.BlockItem;
import ast.CompoundStatement;
import ast.ExpressionStatement;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeExpressionStatement extends PDGNode<ExpressionStatement> {

    public PDGNodeExpressionStatement(ExpressionStatement expressionStatement) {
        this(expressionStatement, false);
    }

    public PDGNodeExpressionStatement(ExpressionStatement expressionStatement, boolean required) {
        super(expressionStatement, required);
    }

    @Override
    public ExpressionStatement sort(PDGSorter sorter) {
        return blockItem;
    }
}
