package pdg;

import ast.Expression;
import ast.JumpBreakStatement;
import ast.Statement;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeBreak extends PDGNode<JumpBreakStatement> {

    public PDGNodeBreak(JumpBreakStatement statement) {
        this(statement, false);
    }

    public PDGNodeBreak(JumpBreakStatement statement, boolean required) {
        super(statement, required);
    }

    public JumpBreakStatement sort(PDGSorter sorter) {
        return blockItem;
    }
}
