package pdg;

import ast.statement.impl.LabeledDefaultStatement;
import lombok.EqualsAndHashCode;
import lombok.Value;

/**
 * Created by ooee on 12/1/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeDefaultCase extends PDGNode<LabeledDefaultStatement> {
    public PDGNodeDefaultCase(LabeledDefaultStatement statement) {
        super(statement);
    }

    @Override
    public LabeledDefaultStatement sort(PDGSorter sorter) {
        return blockItem;
    }
}
