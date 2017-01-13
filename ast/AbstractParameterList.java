package ast;

import lombok.AllArgsConstructor;
import lombok.Value;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
@Value
@AllArgsConstructor
public class AbstractParameterList implements BaseElement {
    private final List<? extends AbstractParameter> parameters;

    public AbstractParameterList() {
        parameters = Collections.emptyList();
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        return toCode();
    }

    public String toCode() {
        List<String> codes = parameters.stream().map(para -> para.toCode()).collect(Collectors.toList());
        return String.join(", ", codes);
    }
}
