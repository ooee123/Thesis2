package pdg;

import ast.BlockItem;
import ast.Expression;
import ast.IterationStatementWhile;
import ast.Statement;
import lombok.Value;

import java.util.List;

/**
 * Created by ooee on 10/9/16.
 */
@Value
public class PDGNodeWhile extends PDGNode<IterationStatementWhile> {
    private PDGNode<? extends Statement> body;

    public PDGNodeWhile(IterationStatementWhile statement, PDGNode<? extends Statement> body) {
        this(statement, body, false);
    }

    public PDGNodeWhile(IterationStatementWhile statement, PDGNode<? extends Statement> body, boolean required) {
        super(statement, required);
        this.body = body;
    }

    public IterationStatementWhile sort(PDGSorter sorter) {
        Expression condition = blockItem.getCondition();
        Statement statement = body.sort(sorter);
        return new IterationStatementWhile(condition, statement);
    }
}
