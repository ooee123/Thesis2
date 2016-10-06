package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class SelectionStatementSwitch implements SelectionStatement {
    @NonNull private Expression expression;
    @NonNull private Statement statement;

    @Override
    public String toCode() {
        return String.format("switch (%s) %s", expression.toCode(), statement.toCode());
    }
}