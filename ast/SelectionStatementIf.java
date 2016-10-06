package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import lombok.RequiredArgsConstructor;

/**
 * Created by ooee on 9/25/16.
 */
@Data
public class SelectionStatementIf implements SelectionStatement {
    @NonNull private Expression condition;
    @NonNull private Statement thenStatement;
    private Statement elseStatement;

    public SelectionStatementIf(Expression condition, Statement thenStatement, Statement elseStatement) {
        this.condition = condition;
        this.thenStatement = thenStatement;
        this.elseStatement = elseStatement;
    }

    public SelectionStatementIf(Expression condition, Statement thenStatement) {
        this(condition, thenStatement, null);
    }

    @Override
    public String toCode() {
        String base = String.format("if (%s) %s", condition.toCode(), thenStatement.toCode());
        if (elseStatement != null) {
            return String.format("%s else %s", base, elseStatement.toCode());
        } else {
            return base;
        }
    }
}
