package pdg;

import ast.ExpressionStatement;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
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
