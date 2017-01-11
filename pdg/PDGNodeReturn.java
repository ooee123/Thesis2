package pdg;

import ast.statement.impl.JumpReturnStatement;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
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
}
