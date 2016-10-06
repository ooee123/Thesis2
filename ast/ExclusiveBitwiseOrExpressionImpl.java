package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ExclusiveBitwiseOrExpressionImpl implements ExclusiveBitwiseOrExpression {
    public static final String BITWISE_OR = "^";

    private Collection<BitwiseAndExpression> bitwiseAndExpressions;

    @Override
    public String toCode() {
        List<String> codes = bitwiseAndExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(BITWISE_OR, codes);
    }
}
