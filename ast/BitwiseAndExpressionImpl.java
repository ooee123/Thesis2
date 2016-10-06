package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class BitwiseAndExpressionImpl implements BitwiseAndExpression {

    public static final String BITWISE_AND = "&";

    private Collection<EqualityExpression> equalityExpressions;

    @Override
    public String toCode() {
        List<String> equalityCodes = equalityExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(BITWISE_AND, equalityCodes);
    }
}
