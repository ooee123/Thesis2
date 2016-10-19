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
    private PDGNode body;

    public PDGNodeWhile(IterationStatementWhile statement, PDGNode<? extends Statement> body) {
        this(statement, body, false);
    }

    public PDGNodeWhile(IterationStatementWhile statement, PDGNode<? extends Statement> body, boolean required) {
        super(statement, required);
        this.body = body;
    }

    public IterationStatementWhile sort(PDGSorter sorter) {
        Expression condition = blockItem.getCondition();
        Statement statement;
        if (body instanceof PDGNodeCompoundStatement) {
            statement = sorter.sort(((PDGNodeCompoundStatement) body).getBody());
        } else {
            statement = ((Statement) body.blockItem);
        }
        return new IterationStatementWhile(condition, statement);
    }

    public String toCode(PDGSorterDefault sorter) {

    }
}
