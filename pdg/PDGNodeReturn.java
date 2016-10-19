package pdg;

import ast.Expression;
import ast.IterationStatementDoWhile;
import ast.JumpReturnStatement;
import ast.Statement;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeReturn extends PDGNode<JumpReturnStatement> {

    public PDGNodeReturn(JumpReturnStatement statement) {
        this(statement, false);
    }

    public PDGNodeReturn(JumpReturnStatement statement, boolean required) {
        super(statement, required);
    }

    public JumpReturnStatement sort(PDGSorter sorter) {
        return new JumpReturnStatement(blockItem.getReturnExpression());
    }

    public String toCode(PDGSorterDefault sorter) {
        return "";
    }
}
