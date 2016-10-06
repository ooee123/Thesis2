package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class LogicalOrExpressionImpl implements LogicalOrExpression {
    public static final String OR = "||";

    private Collection<LogicalAndExpression> logicalAndExpressions;

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(logicalAndExpressions);
        return String.join(OR, codes);
    }
}
