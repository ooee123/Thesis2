package pdg;

import ast.Expression;
import ast.IterationStatementWhile;
import ast.Statement;
import com.google.common.collect.Lists;
import lombok.EqualsAndHashCode;
import lombok.NonNull;
import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeWhile extends PDGNodeContainsStatementNode<IterationStatementWhile> {
    @NonNull private PDGNode<? extends Statement> body;

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

    @Override
    public Collection<PDGNode<? extends Statement>> getStatementNodes() {
        return Lists.newArrayList(body);
    }
}
