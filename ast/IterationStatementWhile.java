package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementWhile implements IterationStatement {
    @NonNull private Expression condition;
    private Statement statement;

    @Override
    public String toCode() {
        return String.format("while (%s) %s", condition.toCode(), statement.toCode());
    }
}
