package ast;

import lombok.Value;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class InclusiveBitwiseOrExpressionImpl implements InclusiveBitwiseOrExpression {
    public static final String INCLUSIVE_BITWISE_OR = "|";

    private Collection<ExclusiveBitwiseOrExpression> exclusiveBitwiseOrExpressions;

    @Override
    public String toCode() {
        List<String> codes = exclusiveBitwiseOrExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(INCLUSIVE_BITWISE_OR, codes);
    }
}
