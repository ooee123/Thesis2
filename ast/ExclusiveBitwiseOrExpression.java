package ast;

import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ExclusiveBitwiseOrExpression implements Expression {
    private Collection<BitwiseAndExpression> bitwiseAndExpressions;
}
