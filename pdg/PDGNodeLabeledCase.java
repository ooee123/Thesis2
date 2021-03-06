package pdg;

import ast.statement.impl.LabeledCaseStatement;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 12/1/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeLabeledCase extends PDGNode<LabeledCaseStatement> {
    public PDGNodeLabeledCase(LabeledCaseStatement statement) {
        super(statement);
    }

    @Override
    public LabeledCaseStatement sort(PDGSorter sorter) {
        return blockItem;
    }
}
