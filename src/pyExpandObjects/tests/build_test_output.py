import os
import pandas as pd

base_project_path = os.path.dirname(
    os.path.dirname(
        os.path.abspath(__file__)
    )
)


def make_table(df):
    html_tables = {}
    df[['DocSection', 'DocText']] = df["DocText"].str.rsplit(":", 1, expand=True)
    for section, sub_df in df.groupby(['DocSection']):
        sub_df.drop(['DocSection'], axis=1, inplace=True)
        html_text = ''
        html_text += '<h4>' + section.replace(":", " : ") + "</h4>"
        sub_df.rename(columns={
            "DocText": "Test",
            "TimeStamp": "Last Check",
            "FunctionStatus": "Status"
        }, inplace=True)
        html_text += sub_df.to_html(index=False) + '\n'
        html_tables[section.strip()] = html_text
    return html_tables


def main():
    df = pd.read_csv(
        os.path.join(base_project_path, "logs", "test.log"),
        header=None,
        names=["TimeStamp", "DocText", "FileName", "FunctionName", "FunctionStatus"])
    # Get latest function return
    df = df\
        .sort_values(['TimeStamp'], ascending=True)\
        .groupby(['DocText', 'FileName', 'FunctionName'])\
        .last()\
        .reset_index()
    html_text = make_table(df)
    sections = sorted([i for i in html_text.keys()])
    # Push the general section to the top
    if "General" in sections:
        sections.remove("General")
        sections.insert(0, "General")
    # create html file and save to docs static folder.
    # write out all testing sections
    with open(os.path.join(base_project_path, "docs", "_static", "testing_output.html"), 'w') as f:
        f.write("""
            <!DOCTYPE html>
            <html>
            <head>
            <title>Unittest Results</title>
        """)
        with open(os.path.join(base_project_path, "docs", "_static", "testing_base.html"), 'r') as f2:
            notes_data = f2.read()
        f.write(notes_data)
        for section in sections:
            if not section.startswith('Simulation'):
                f.write(html_text[section])
        f.write("""
            </head>
            <body>
        """)
    with open(os.path.join(base_project_path, "docs", "_static", "simulation_output.html"), 'w') as f:
        # write out all full simulation sections
        f.write("""
            <!DOCTYPE html>
            <html>
            <head>
            <title>Simulation Test Results</title>
        """)
        with open(os.path.join(base_project_path, "docs", "_static", "testing_base.html"), 'r') as f2:
            notes_data = f2.read()
        f.write(notes_data)
        for section in sections:
            if section.startswith('Simulation'):
                f.write(html_text[section])
        f.write("""
            </head>
            <body>
        """)
    return


if __name__ == "__main__":
    main()
