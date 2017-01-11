package pdg;

import ast.statement.impl.JumpBreakStatement;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
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
