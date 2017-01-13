package ast;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import lombok.Value;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/24/16.
 */
@Value
@AllArgsConstructor
public class ParameterList implements BaseElement {
    private final List<Parameter> parameters;

    public String toCode() {
        List<String> codes = parameters.stream().map(para -> para.toCode()).collect(Collectors.toList());
        return String.join(", ", codes);
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        return toCode();
    }
}
