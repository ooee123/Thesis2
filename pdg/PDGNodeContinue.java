package pdg;

import ast.Expression;
import ast.JumpContinueStatement;
import ast.Statement;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeContinue extends PDGNode<JumpContinueStatement> {

    public PDGNodeContinue(JumpContinueStatement statement) {
        this(statement, false);
    }

    public PDGNodeContinue(JumpContinueStatement statement, boolean required) {
        super(statement, required);
    }

    public JumpContinueStatement sort(PDGSorter sorter) {
        return blockItem;
    }
}
