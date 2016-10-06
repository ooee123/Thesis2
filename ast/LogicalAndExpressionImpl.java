package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class LogicalAndExpressionImpl implements LogicalAndExpression {
    public static final String AND = "&&";

    private Collection<InclusiveBitwiseOrExpression> inclusiveBitwiseOrExpressions;

    @Override
    public String toCode() {
        List<String> codes = collectionToCode(inclusiveBitwiseOrExpressions);
        return String.join(AND, codes);
    }
}
